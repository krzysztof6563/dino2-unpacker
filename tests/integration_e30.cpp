// End-to-end test of the glTF animation export on E30.DAT (C++ port of Ziggertron's
// integration_e30.py). Runs the unpacker on a copy of E30.DAT and checks:
//   - 11 clips, 20 joints, 543 keys in all
//   - geometry, skin and every rotation key match a reference export, when one is given
//   - --animation-tps=30 doubles every key time
//   - --in-place keeps rotations and body position but drops movement through the world
//   - --no-animations writes the same geometry without clips
//   - invalid --animation-tps values are rejected
// The root translation is not compared with the reference: since commit 224c314 it follows
// each frame's stored body position (the reference keeps the body at bind height).
//
// Usage: integration_e30 UNPACKER E30.DAT [REFERENCE.gltf|REFERENCE.glb]
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Gltf {
    QJsonObject json;
    QByteArray buffer;
};

void check(bool value, const std::string& message) {
    if (!value) throw std::runtime_error(message);
}

Gltf load(const QString& path) {
    QFile file(path);
    check(file.open(QIODevice::ReadOnly), "cannot open " + path.toStdString());
    const QByteArray data = file.readAll();
    Gltf g;
    if (data.startsWith("glTF")) {                         // binary .glb: JSON chunk, then BIN chunk
        quint32 length = 0;
        std::memcpy(&length, data.constData() + 12, 4);
        g.json = QJsonDocument::fromJson(data.mid(20, length)).object();
        g.buffer = data.mid(28 + length);
    } else {                                               // .gltf with an embedded data: URI
        g.json = QJsonDocument::fromJson(data).object();
        const QString uri = g.json["buffers"].toArray()[0].toObject()["uri"].toString();
        g.buffer = QByteArray::fromBase64(uri.section(',', 1).toLatin1());
    }
    check(!g.json.isEmpty(), "not a glTF file: " + path.toStdString());
    return g;
}

std::vector<double> read(const Gltf& g, int index) {
    const QJsonObject a = g.json["accessors"].toArray()[index].toObject();
    const QJsonObject v = g.json["bufferViews"].toArray()[a["bufferView"].toInt()].toObject();
    const QString type = a["type"].toString();
    const int width = type == "SCALAR" ? 1 : type == "VEC2" ? 2 : type == "VEC3" ? 3 : type == "VEC4" ? 4 : 16;
    const int component = a["componentType"].toInt();
    const int size = component == 5123 ? 2 : 4;
    const qsizetype count = qsizetype(a["count"].toInt()) * width;
    const qsizetype start = v["byteOffset"].toInt() + a["byteOffset"].toInt();
    check(start >= 0 && start + count * size <= g.buffer.size(), "accessor outside its buffer");
    std::vector<double> values;
    values.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
        const char* p = g.buffer.constData() + start + i * size;
        if (component == 5126) { float f; std::memcpy(&f, p, 4); values.push_back(f); }
        else if (component == 5123) { quint16 u; std::memcpy(&u, p, 2); values.push_back(u); }
        else if (component == 5125) { quint32 u; std::memcpy(&u, p, 4); values.push_back(u); }
        else throw std::runtime_error("unexpected component type");
    }
    return values;
}

void same(const std::vector<double>& a, const std::vector<double>& b, const std::string& what) {
    check(a.size() == b.size(), what + ": different lengths");
    for (std::size_t i = 0; i < a.size(); ++i)
        check(std::abs(a[i] - b[i]) < 2e-6, what + ": values differ");
}

QJsonArray animations(const Gltf& g) { return g.json["animations"].toArray(); }
QJsonObject animation(const Gltf& g, int i) { return animations(g)[i].toObject(); }
QJsonObject channel(const QJsonObject& a, int i) { return a["channels"].toArray()[i].toObject(); }
QJsonObject sampler(const QJsonObject& a, const QJsonObject& c) {
    return a["samplers"].toArray()[c["sampler"].toInt()].toObject();
}

class Unpacker {
public:
    Unpacker(QString exe, QString input) : exe(std::move(exe)), input(std::move(input)) {}
    int exitCode(const QStringList& options) const {
        QProcess p;
        p.start(exe, options + QStringList{input});
        p.waitForFinished(-1);
        return p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
    }
    Gltf run(const QStringList& options = {}) const {
        QProcess p;
        p.start(exe, options + QStringList{input});
        p.waitForFinished(-1);
        check(p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0,
              "unpacker failed:\n" + p.readAll().toStdString());
        return load(input + ".model.0.gltf");
    }
private:
    QString exe, input;
};

void test(const QString& exe, const QString& dat, const QString& reference) {
    QTemporaryDir work;
    check(work.isValid(), "cannot create a temporary folder");
    const QString input = work.filePath("E30.DAT");
    check(QFile::copy(dat, input), "cannot copy " + dat.toStdString());
    const Unpacker unpacker(QFileInfo(exe).absoluteFilePath(), input);

    const Gltf g = unpacker.run();
    check(animations(g).size() == 11, "expected 11 E30 clips");
    check(g.json["skins"].toArray()[0].toObject()["joints"].toArray().size() == 20, "expected 20 joints");
    int keys = 0;
    for (int i = 0; i < animations(g).size(); ++i) {
        const QJsonObject a = animation(g, i);
        keys += g.json["accessors"].toArray()[sampler(a, channel(a, 0))["input"].toInt()].toObject()["count"].toInt();
    }
    check(keys == 543, "expected 543 keys in all");

    if (!reference.isEmpty()) {
        const Gltf r = load(reference);
        for (int i = 0; i < 7; ++i) same(read(g, i), read(r, i), "geometry accessor " + std::to_string(i));
        check(animations(g).size() == animations(r).size(), "clip count differs from the reference");
        for (int i = 0; i < animations(g).size(); ++i) {
            const QJsonObject a = animation(g, i), ra = animation(r, i);
            check(a["channels"].toArray().size() == 21 && ra["channels"].toArray().size() == 21,
                  "expected 20 rotation channels and 1 translation channel");
            for (int c = 0; c < 21; ++c) {
                const QJsonObject ac = channel(a, c), rc = channel(ra, c);
                check(ac["target"] == rc["target"], "channel targets differ from the reference");
                const QJsonObject s = sampler(a, ac), rs = sampler(ra, rc);
                same(read(g, s["input"].toInt()), read(r, rs["input"].toInt()), "key times");
                if (ac["target"].toObject()["path"].toString() == "rotation")
                    same(read(g, s["output"].toInt()), read(r, rs["output"].toInt()), "rotation keys");
            }
        }
    }

    const Gltf slow = unpacker.run({"--animation-tps=30"});
    for (int i = 0; i < animations(g).size(); ++i) {
        const QJsonObject a = animation(g, i), s = animation(slow, i);
        std::vector<double> doubled = read(g, sampler(a, channel(a, 0))["input"].toInt());
        for (double& t : doubled) t *= 2;
        same(doubled, read(slow, sampler(s, channel(s, 0))["input"].toInt()), "--animation-tps=30 key times");
    }

    const Gltf stationary = unpacker.run({"--in-place"});
    for (int i = 0; i < animations(g).size(); ++i) {
        const QJsonObject a = animation(g, i), s = animation(stationary, i);
        check(s["channels"].toArray().size() == 21, "--in-place: expected 21 channels");
        for (int c = 0; c < 21; ++c) {
            const QJsonObject ac = channel(a, c), sc = channel(s, c);
            check(ac["target"] == sc["target"], "--in-place: channel targets changed");
            const std::vector<double> moving = read(g, sampler(a, ac)["output"].toInt());
            const std::vector<double> still = read(stationary, sampler(s, sc)["output"].toInt());
            if (ac["target"].toObject()["path"].toString() == "rotation") {
                same(moving, still, "--in-place rotation keys");
            } else {
                // Same body height; the movement through the world is horizontal only.
                check(moving.size() == still.size(), "--in-place translation length");
                for (std::size_t k = 1; k < moving.size(); k += 3)
                    check(std::abs(moving[k] - still[k]) < 2e-6, "--in-place changed the body height");
            }
        }
    }

    const Gltf rigOnly = unpacker.run({"--no-animations"});
    check(!rigOnly.json.contains("animations"), "--no-animations still wrote clips");
    for (int i = 0; i < 7; ++i) same(read(rigOnly, i), read(g, i), "--no-animations geometry");

    for (const char* value : {"0", "-1", "nan", "inf", "60junk"})
        check(unpacker.exitCode({QString("--animation-tps=") + value}) != 0,
              std::string("--animation-tps=") + value + " was accepted");
}

}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: integration_e30 UNPACKER E30.DAT [REFERENCE.gltf|REFERENCE.glb]\n";
        return 2;
    }
    try {
        test(QString::fromLocal8Bit(argv[1]), QString::fromLocal8Bit(argv[2]),
             argc == 4 ? QString::fromLocal8Bit(argv[3]) : QString());
    } catch (const std::exception& error) {
        std::cerr << "E30 integration FAILED: " << error.what() << '\n';
        return 1;
    }
    std::cout << "E30 integration passed: 11 clips, 20 joints and 543 keys"
              << (argc == 4 ? "; geometry, skin and every rotation key match the reference" : "")
              << "; options passed.\n";
    return 0;
}
