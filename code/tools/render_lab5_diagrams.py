"""Render deterministic explanatory diagrams; these are not screenshots."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "doc/lab5/images"
OUT.mkdir(parents=True, exist_ok=True)
FONT = Path("C:/Windows/Fonts/msyh.ttc")
if not FONT.exists():
    FONT = Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")
def canvas(title, height):
    im = Image.new("RGB", (1500,height), "white")
    d = ImageDraw.Draw(im)
    d.text((55,28),title,font=ImageFont.truetype(str(FONT),36),fill="black")
    return im,d
def text(d,xy,s,size=27):
    d.multiline_text(xy,s,font=ImageFont.truetype(str(FONT),size),fill="black",spacing=8)
def box(d,rect,s):
    d.rounded_rectangle(rect,radius=12,outline="black",width=2)
    text(d,(rect[0]+18,rect[1]+16),s)
def arrow(d,a,b):
    d.line((a,b),fill="black",width=3)
    x,y=b
    if abs(b[0]-a[0])>=abs(b[1]-a[1]):
        sign=1 if b[0]>a[0] else -1
        d.polygon([(x,y),(x-14*sign,y-8),(x-14*sign,y+8)],fill="black")
    else:
        d.polygon([(x,y),(x-8,y-14),(x+8,y-14)],fill="black")
im,d=canvas("Lab5 COW 同一数据页的三个时刻",1060)
for i,title in enumerate(["① fork 完成：共享，只读", "② 子先写：复制一次", "③ 父后写：独占恢复，零复制"]):
    y=105+i*300
    text(d,(55,y),title,30)
    box(d,(65,y+70,480,y+150),"父 PTE  W="+str(i==2 and 1 or 0)+"  COW="+str(i!=2 and 1 or 0))
    box(d,(65,y+175,480,y+255),"子 PTE  W="+str(i>0 and 1 or 0)+"  COW="+str(i==0 and 1 or 0))
    if i==0:
        box(d,(900,y+115,1415,y+215),"物理页 PA0   refcnt = 2")
        arrow(d,(480,y+110),(900,y+145));arrow(d,(480,y+215),(900,y+185))
    else:
        box(d,(900,y+70,1415,y+150),"物理页 PA0   refcnt = 1")
        box(d,(900,y+175,1415,y+255),"物理页 PA1   refcnt = 1")
        arrow(d,(480,y+110),(900,y+110));arrow(d,(480,y+215),(900,y+215))
text(d,(55,1010),"说明图；只读代码不标 COW，TRAPFRAME / TRAMPOLINE 不参加此流程。",24)
im.save(OUT/"lab5-cow-snapshots.png")
im,d=canvas("Lab5 exec 参数栈：echo hi 的实际装配顺序",1030)
text(d,(55,95),"stack_top = 0x5000；栈向低地址增长；字符串先 argv[0] 后 argv[1]。",26)
rows=[("0x4ff0..0x4fff",'"echo\\0" + 对齐空间；argv[0] → 0x4ff0'),("0x4fe0..0x4fef",'"hi\\0" + 对齐空间；argv[1] → 0x4fe0'),("0x4fd8..0x4fdf","8 字节对齐填充"),("0x4fd0..0x4fd7","argv[2] = NULL 哨兵"),("0x4fc8..0x4fcf","argv[1] = 0x4fe0"),("0x4fc0..0x4fc7","argv[0] = 0x4ff0  ← sp = a1 = 0x4fc0")]
for i,(a,b) in enumerate(rows):
    y=160+i*110
    box(d,(65,y,1430,y+92),a+"     "+b)
text(d,(65,865),"a0 = argc = 2，由 exec 成功返回值经 syscall 写入。",30)
text(d,(65,925),"这是按本实现地址和算法绘制的布局图；非终端截图或内存转储。",25)
im.save(OUT/"lab5-exec-stack.png")
im,d=canvas("Lab5 缺页与内核 copyout 共用页面分离",1060)
box(d,(70,110,670,220),"用户 store fault：scause=15")
box(d,(830,110,1430,220),"copyout / 写范围预检")
arrow(d,(370,220),(650,295));arrow(d,(1130,220),(850,295))
box(d,(350,295,1150,410),"cow_alloc：va < MAXVA，PTE_V / U / R")
arrow(d,(750,410),(750,460))
box(d,(350,460,1150,550),"无 W 且无 COW → 拒绝；已有 W → 成功")
arrow(d,(750,550),(750,600))
box(d,(65,600,710,755),"refcnt = 1\n原地置 W，清 COW；不增加 pmc(0)")
box(d,(790,600,1430,755),"refcnt > 1\n分配、复制、替换 PTE，再减旧引用")
arrow(d,(550,755),(650,835));arrow(d,(1000,755),(850,835))
box(d,(350,835,1150,930),"sfence.vma；恢复用户执行 / 内核复制")
text(d,(65,985),"OOM：原 PTE/refcnt 保持；用户 fault 退出 -1，copyout 返回 -1。",25)
im.save(OUT/"lab5-cow-path.png")
