#!/usr/bin/env python3
"""Real QEMU disk filesystem tests; isolate fs.img and retain raw UART log."""
import argparse,os,selectors,shutil,subprocess,tempfile,time
from pathlib import Path
CODE=Path(__file__).resolve().parents[1]
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--selftests',action='store_true');args=ap.parse_args()
    with tempfile.TemporaryDirectory(prefix='lab6-') as tmp:
        image=Path(tmp)/'fs.img';shutil.copyfile(CODE/'fs.img',image)
        cmd=['qemu-system-riscv64','-machine','virt','-bios','none','-kernel','kernel/kernel','-m','128M','-smp','1','-nographic','-global','virtio-mmio.force-legacy=false','-drive',f'file={image},if=none,format=raw,id=x0','-device','virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
        p=subprocess.Popen(cmd,cwd=CODE,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,bufsize=0)
        sel=selectors.DefaultSelector();sel.register(p.stdout,selectors.EVENT_READ);out=bytearray()
        log=CODE/('lab6-selftest.log' if args.selftests else 'lab6-qemu.log')
        with log.open('wb') as f:
            def wait(start,needle,timeout=90):
                until=time.monotonic()+timeout
                while time.monotonic()<until:
                    for key,_ in sel.select(.1):
                        b=os.read(key.fd,8192)
                        if not b: raise RuntimeError('QEMU exited')
                        out.extend(b);f.write(b);f.flush()
                    text=out[start:].decode(errors='replace')
                    if 'panic:' in text or 'FAIL' in text:raise RuntimeError(text)
                    if needle in text and text.endswith('sh> '):return text
                raise RuntimeError('timeout: '+out[start:].decode(errors='replace'))
            def send(command,needle):
                start=len(out);p.stdin.write((command+'\n').encode());p.stdin.flush();return wait(start,needle)
            try:
                boot=wait(0,'sh> ')
                assert out.startswith((CODE/'expect_banner.txt').read_bytes())
                assert 'LAB6 disk ELF /sh loaded' in boot
                if args.selftests:assert 'BIO-SELFTEST PASS' in boot and 'LOG-SELFTEST PASS' in boot
                print('[ok] disk ELF boot'+(' and cache/WAL selftests' if args.selftests else ''),flush=True)
                for command,marker in [('fstest','FSTEST PASS'),('fsstress','T1 PASS'),('echo hi','T5-3 PASS')]:
                    text=send(command,marker);assert marker in text;print('[ok] '+command+': '+marker,flush=True)
                for stage in range(6):
                    text=send(f'crashtest {stage}','LAB6 disk ELF /sh loaded')
                    assert f'injecting power-off at stage {stage}' in text
                    assert ('WAL recover header.n=0' in text) == (stage<3)
                    text=send('cat crashfile','sh> ')
                    expected='OLD-CONTENT' if stage<3 else f'STAGE-{stage}-COMMITTED'
                    assert expected in text,(stage,text)
                    print(f'[ok] stage {stage}: {expected}',flush=True)
                # Same disk reboot again proves idempotence after recovery cleared the header.
                text=send('fstest reboot','LAB6 disk ELF /sh loaded');assert 'WAL recover header.n=0' in text
                text=send('cat crashfile','sh> ');assert 'STAGE-5-COMMITTED' in text
                print('[ok] all six reset/recovery stages; UART '+str(log),flush=True)
            finally:
                sel.close();p.kill();p.wait()
if __name__=='__main__':main()
