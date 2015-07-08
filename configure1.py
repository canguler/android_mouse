from Tkinter import *
import Image, ImageTk

buttonNo = None
key1 = None
key2 = None
profFileName = 'profiles.txt'
cfgFileName = 'mouse.cfg'

# Used for taking user input
class TextDialog:
    def __init__(self, parent):
        self.top = Toplevel(parent)
        Label(self.top, text='Enter the profile name.').pack(side=TOP)
        self.e = Entry(self.top)
        self.e.pack(side=TOP)
        b = Button(self.top, text='OK', command=self.onClick)
        b.pack(side=BOTTOM)
    def onClick(self):
        self.value=self.e.get()
        self.top.destroy()

# When user presses a key if a configuration button is already clicked, profile and current configuration is updated
def onKeyPress(event):
    global buttonNo, key1, key2, profileNames, profiles
    print 'Got key press:', event.keycode
    if buttonNo == 1:
        profiles[profile.get()][0] = event.keycode
        buttonNo = None
        button1['text'] = event.keysym
        selectProfile()
        exportProfiles()

    elif buttonNo == 2:
        profiles[profile.get()][1] = event.keycode
        buttonNo = None
        button2['text'] = event.keysym
        selectProfile()
        exportProfiles()

# Updates profiles file
def exportProfiles():
    profFile = open(profFileName,'w')
    for i in profiles:
        profFile.write(i + ' ')
        profFile.write(str(profiles[i][0]-8) + ' ')
        profFile.write(str(profiles[i][1]-8) + '\n')
    profFile.close()

# Switches button states
def button1OnClick():
    global buttonNo
    if buttonNo is None:
        buttonNo = 1
        button1['text'] = 'press a button'

def button2OnClick():
    global buttonNo
    if buttonNo is None:
        buttonNo = 2
        button2['text'] = 'press a button'

def newProfile():
    d = TextDialog(tkroot)
    tkroot.wait_window(d.top)
    n = d.value
    profileNames.append(n)
    profiles[n] = [0,0]
    profileSelect['menu'].add_command(label=n, command=lambda value=n: profile.set(value))

def onProfileChange(*args):
    button1['text'] = profiles[profile.get()][0]
    button2['text'] = profiles[profile.get()][1]
    selectProfile()

def selectProfile():
    print profile.get()
    f = open(cfgFileName, 'w')
    f.write(str(profiles[profile.get()][0]-8) + '\n')
    f.write(str(profiles[profile.get()][1]-8) + '\n')
    f.close()

tkroot = Tk()
profileNames = []
profiles = {}
profile = StringVar()
profile.trace('w', onProfileChange)

profFile = open(profFileName, 'r')
for line in profFile:
    print line
    n,k1,k2= line.split()
    profileNames.append(n)
    profiles[n] = [int(k1)+8,int(k2)+8]
profFile.close()

profileSelect = OptionMenu(tkroot, profile, *profileNames)
profileSelect.pack(side=TOP)

addButton = Button(tkroot, text='New Profile', command=newProfile)
addButton.pack(side=TOP)

button1 = Button(tkroot, text='Custom 1', command=button1OnClick)
img = Image.open('mouse.jpg')
img = ImageTk.PhotoImage(img.resize((250, 250)))
widget = Label(tkroot, image=img)
widget.pack(side=TOP)
button1.pack(side=LEFT, fill=BOTH)
button2 = Button(tkroot, text='Custom 2', command=button2OnClick)
button2.pack(side=RIGHT, fill=BOTH)
tkroot.bind('<KeyPress>', onKeyPress)
tkroot.focus()
tkroot.title('Profile Manager')
tkroot.mainloop()
