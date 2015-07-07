from Tkinter import *
import Image, ImageTk

buttonNo = None
key1 = None
key2 = None
def showPosEvent(event):
    print 'Widget=%s X=%s Y=%s' % (event.widget, event.x, event.y)

def onKeyPress(event):
    global buttonNo, key1, key2
    print 'Got key press:', event.keycode
    if buttonNo == 1:
        key1 = event.keycode
        buttonNo = None
        button1["text"] = event.keysym
        f = open('mouse.cfg', 'w')
        f.write(str(key1-8) + '\n')
        f.write(str(key2-8) + '\n')
        f.close()
    elif buttonNo == 2:
        key2 = event.keycode
        buttonNo = None
        button2["text"] = event.keysym
        f = open('mouse.cfg', 'w')
        f.write(str(key1-8) + '\n')
        f.write(str(key2-8) + '\n')
        f.close()

def button1OnClick():
    global buttonNo
    if buttonNo is None:
        buttonNo = 1
        button1["text"] = "press a button"

def button2OnClick():
    global buttonNo
    if buttonNo is None:
        buttonNo = 2
        button2["text"] = "press a button"

tkroot = Tk()
labelfont = ('courier', 20, 'bold')
f = open('mouse.cfg', 'r')
key1 = int(f.readline().strip()) + 8
key2 = int(f.readline().strip()) + 8
f.close()
button1 = Button(tkroot, text="Custom 1", command=button1OnClick)
img = Image.open("mouse.jpg")
img = ImageTk.PhotoImage(img.resize((250, 250)))
widget = Label(tkroot, image=img)
widget.pack(side=TOP)
button1.pack(side=LEFT, fill=BOTH)
button2 = Button(tkroot, text="Custom 2", command=button2OnClick)
button2.pack(side=RIGHT, fill=BOTH)
tkroot.bind("<KeyPress>", onKeyPress)
tkroot.focus()
tkroot.title('Click Me')
tkroot.mainloop()
