"""Usage: python tests/integration_e30.py UNPACKER E30.DAT VALIDATED_E30.glb"""
import base64
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile


def load(path):
    data = Path(path).read_bytes()
    if data[:4] == b'glTF':
        size = struct.unpack_from('<I', data, 12)[0]
        return json.loads(data[20:20 + size]), data[28 + size:]
    g = json.loads(data)
    return g, base64.b64decode(g['buffers'][0]['uri'].split(',', 1)[1])


def read(g, b, index):
    a = g['accessors'][index]
    v = g['bufferViews'][a['bufferView']]
    width = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}[a['type']]
    kind = {5126: 'f', 5123: 'H', 5125: 'I'}[a['componentType']]
    return struct.unpack_from('<' + kind * (width * a['count']), b,
                              v.get('byteOffset', 0) + a.get('byteOffset', 0))


def same(a, b):
    assert len(a) == len(b)
    assert max((abs(x - y) for x, y in zip(a, b)), default=0) < 2e-6


def main(exe, dat, reference):
    exe = str(Path(exe).resolve())
    rg, rb = load(reference)
    with tempfile.TemporaryDirectory() as work:
        inp = Path(work) / 'E30.DAT'
        shutil.copyfile(dat, inp)
        def run(*options):
            result = subprocess.run([exe, *options, str(inp)], capture_output=True, text=True)
            assert result.returncode == 0, result.stdout + result.stderr
            return load(str(inp) + '.model.0.gltf')
        g, b = run()
        assert len(g['animations']) == 11
        assert len(g['skins'][0]['joints']) == 20
        assert sum(g['accessors'][a['samplers'][0]['input']]['count'] for a in g['animations']) == 543
        for i in range(7):
            same(read(g, b, i), read(rg, rb, i))
        for a, r in zip(g['animations'], rg['animations']):
            assert len(a['channels']) == len(r['channels']) == 21
            for ac, rc in zip(a['channels'], r['channels']):
                assert ac['target'] == rc['target']
                s, rs = a['samplers'][ac['sampler']], r['samplers'][rc['sampler']]
                same(read(g,b,s['input']), read(rg,rb,rs['input']))
                same(read(g,b,s['output']), read(rg,rb,rs['output']))
        slow, sb = run('--animation-tps=30')
        for a, s in zip(g['animations'], slow['animations']):
            same([2*x for x in read(g,b,a['samplers'][0]['input'])],
                 read(slow,sb,s['samplers'][0]['input']))
        stationary, ib = run('--in-place')
        for a, s in zip(g['animations'], stationary['animations']):
            assert len(s['channels']) == 20
            assert all(c['target']['path'] == 'rotation' for c in s['channels'])
            for ac, sc in zip(a['channels'], s['channels']):
                same(read(g,b,a['samplers'][ac['sampler']]['output']),
                     read(stationary,ib,s['samplers'][sc['sampler']]['output']))
        static, bb = run('--no-animations')
        assert 'animations' not in static
        for i in range(7): same(read(static,bb,i),read(g,b,i))
        for value in ('0', '-1', 'nan', 'inf', '60junk'):
            assert subprocess.run([exe,'--animation-tps='+value,str(inp)],capture_output=True).returncode != 0
    print('E30 integration passed: geometry, rig and all 543 motion keys match the validated GLB; options passed.')


if __name__ == '__main__':
    main(*sys.argv[1:])
