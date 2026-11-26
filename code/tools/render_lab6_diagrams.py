"""Deterministic mechanism diagrams; never represented as runtime screenshots."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'doc/lab6/images';OUT.mkdir(parents=True,exist_ok=True)
def font(size):
    for p in ['C:/Windows/Fonts/msyh.ttc','/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf']:
        if Path(p).exists():return ImageFont.truetype(p,size)
    return ImageFont.load_default()
def render(name,title,rows):
    im=Image.new('RGB',(1200,180+len(rows)*130),'white');d=ImageDraw.Draw(im)
    d.text((45,28),title,font=font(30),fill='black')
    d.text((45,80),'机制说明图，非运行截图  |  SID 2024302111427',font=font(20),fill='black')
    for i,row in enumerate(rows):
        y=135+i*130;d.rectangle((45,y,1155,y+95),outline='black',width=2)
        d.text((65,y+14),row[0],font=font(24),fill='black');d.text((65,y+53),row[1],font=font(20),fill='black')
        if i<len(rows)-1:d.line((600,y+95,600,y+128),fill='black',width=2);d.polygon([(594,y+120),(606,y+120),(600,y+129)],fill='black')
    im.save(OUT/(name+'.png'))
render('lab6-wal','WAL 四步提交与六阶段恢复',[
 ('修改缓存并固定：log_write → bpin；同块写吸收','Stage 0：新事务之前复位；begin_op 最多准入 8 个固定块，预留 3 槽'),
 ('步骤 1：write_log，将修改写入连续日志数据块','Stage 1：日志写到一半；Stage 2：日志写完但尚无提交头'),
 ('步骤 2：非零 header.n 落盘 —— 原子提交点','Stage 3：提交点之后复位；启动恢复必须重放'),
 ('步骤 3：install_trans，将日志安装至原位并解除固定','Stage 4：原位安装到一半；Stage 5：安装完成但还未清头'),
 ('步骤 4：header.n = 0 落盘，事务结束','Stage 0–2：OLD-CONTENT；Stage 3–5：STAGE-X-COMMITTED')])
render('lab6-disk-exec','磁盘 ELF 加载与文件系统分层',[
 ('用户 exec("echo", argv) → sys_exec → proc_exec','提取用户字符串；begin_op；namei 逐级查目录'),
 ('inode → readi → bread → VirtIO → fs.img','11 槽 LFU 缓存；缓存身份由全局自旋锁保护，数据由睡眠锁保护'),
 ('校验 ELF 与 PT_LOAD，建立新页表并读入文件段','只读可执行代码；可写数据；保护页洞；1 页用户栈'),
 ('压入 argv 字符串、NULL 哨兵与指针数组','sp 按 16 字节对齐；a0 = argc；a1 = argv；epc = ELF entry'),
 ('提交新映像并释放旧页表；失败时完整回滚','首个 forkret 开中断后挂载；fork 复制 fd/cwd 引用，exit 关闭')])
