#!/usr/bin/env python
'''Application main window

Demonstrates a typical application window, with menubar, toolbar, statusbar.'''
# pygtk version: Maik Hertha <maik.hertha@berlin.de>
import pygtk
pygtk.require('2.0')
import gobject
import webkit
import gtk
import gtk.glade
import os
import base64
import pythonSimpleWebsocket
from pythonSimpleWebsocket import WebSocketServer
#import editorWebSocket as websocket
#from tornado import web, ioloop
import threading
import signal
import subprocess
import shlex
import ntpath
import EntryDialog

ALIVE = True

#last action performed
lastAction = ""
#list of open files
openFiles = []

#preferencesDialog.signal_autoconnect({gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT})


#current working directory for webkit
currentWorkingDirectory = "file://"+os.getcwd()

#editor webpage
editorWindowWebPage = currentWorkingDirectory+"/editor/editor.html"
#editor welcome page
welcomeWebPage = currentWorkingDirectory+"/editor/welcomepage.html"

#editor window


def signal_handler(signal, frame):
    global ALIVE
    ALIVE = false

signal.signal(signal.SIGINT, signal_handler)

(
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE
) = range(3)

(
  SHAPE_SQUARE,
  SHAPE_RECTANGLE,
  SHAPE_OVAL,
) = range(3)

ui_info = \
'''<ui>
  <menubar name='MenuBar'>
    <menu action='FileMenu'>
      <menuitem action='New'/>
      <menuitem action='Open'/>
      <menuitem action='Save'/>
      <menuitem action='SaveAs'/>
      <separator/>
    </menu>
    <menu action='Basic4GLMenu'>
        <menuitem action='Preferences'/>
    </menu>
  </menubar>
  <toolbar  name='ToolBar'>
    <toolitem action='New' />
    <separator/>
    <toolitem action='Open'/>
    <toolitem action='Save' />
    <separator/>
    <toolitem action='Run'/>
    <toolitem action='Kill'/>
  </toolbar>
</ui>'''

class ApplicationMainWindowDemo(gtk.Window):
    webview = webkit.WebView()
    preferencesDialog = None
    builder = None
    def __init__(self, parent=None):
        #register_stock_icons()

        # Create the toplevel window
        gtk.Window.__init__(self)
        try:
            self.set_screen(parent.get_screen())
        except AttributeError:
            self.connect('destroy', lambda *w: gtk.main_quit())

        self.set_title(self.__class__.__name__)
        self.set_default_size(500,500)

        merge = gtk.UIManager()
        self.set_data("ui-manager", merge)
        merge.insert_action_group(self.__create_action_group(), 0)
        self.add_accel_group(merge.get_accel_group())

        try:
            mergeid = merge.add_ui_from_string(ui_info)
        except gobject.GError, msg:
            print "building menus failed: %s" % msg
        bar = merge.get_widget("/MenuBar")
        bar.show()

        table = gtk.Table(1, 4, False)
        self.add(table)

        table.attach(bar,
            # X direction #          # Y direction
            0, 1,                      0, 1,
            gtk.EXPAND | gtk.FILL,     0,
            0,                         0);

        bar = merge.get_widget("/ToolBar")
        bar.set_tooltips(True)
        bar.show()
        table.attach(bar,
            # X direction #       # Y direction
            0, 1,                   1, 2,
            gtk.EXPAND | gtk.FILL,  0,
            0,                      0)

        # Create document
        sw = gtk.ScrolledWindow()
        sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        sw.set_shadow_type(gtk.SHADOW_IN)

        table.attach(sw,
            # X direction           Y direction
            0, 1,                   2, 3,
            gtk.EXPAND | gtk.FILL,  gtk.EXPAND | gtk.FILL,
            0,                      0)

        #setup the webkit element in the webpage
        self.webview.load_uri(welcomeWebPage);
        self.webview.grab_focus()

        
			# TODO get builder working! need GTK+3 for glade, or embed it without glade
		  #self.builder = gtk.Builder()
        #self.builder.add_from_file(os.getcwd()+"/preferencesDialog.glade")

        #self.preferencesDialog = builder.get_object("dialog1")


        sw.add (self.webview)

        # Create statusbar
        self.statusbar = gtk.Statusbar()
        table.attach(self.statusbar,
            # X direction           Y direction
            0, 1,                   3, 4,
            gtk.EXPAND | gtk.FILL,  0,
            0,                      0)

        # Show text widget info in the statusbar
        mark_set_callback = (lambda buffer, new_location, mark:
            self.update_statusbar(buffer))

        # cursor moved

        self.connect("window_state_event", self.update_resize_grip)

        self.show_all()



    def __create_action_group(self):
        # GtkActionEntry
        entries = (
          ( "FileMenu", None, "_File" ),               # name, stock id, label
          ( "Basic4GLMenu", None, "_Basic4GL" ), # name, stock id, label
          ( "Preferences", None, "_Preferences", "<control>P", "Change preferences", self.activate_action  ),            # name, stock id, label
          ( "New", gtk.STOCK_FILE, "_New", "<control>N", "Create a file", self.activate_action ),
          ( "Open", gtk.STOCK_OPEN,"_Open","<control>O", "Open a file",  self.activate_action ),
          ( "Save", gtk.STOCK_SAVE,"_Save","<control>S", "Save current file",self.activate_action ),
          ( "SaveAs", gtk.STOCK_SAVE,"Save _As...", None, "Save to a file",self.activate_action ),
          ( "About", None,"_About", "<control>A", "About",self.activate_about ),
          ( "Run", gtk.STOCK_MEDIA_PLAY,"_Run", None,"Run compiler",self.activate_action ),
          ( "Kill", gtk.STOCK_MEDIA_STOP,"ST_OP", None, "Kill Program", self.activate_action)
          );

        # GtkToggleActionEntry
        toggle_entries = (
          ( "Bold", gtk.STOCK_BOLD,                    # name, stock id
             "_Bold", "<control>B",                    # label, accelerator
            "Bold",                                    # tooltip
            self.activate_action,
            True ),                                    # is_active
        )

        # GtkRadioActionEntry
        color_entries = (
          ( "Red", None,                               # name, stock id
            "_Red", "<control><shift>R",               # label, accelerator
            "Blood", COLOR_RED ),                      # tooltip, value
          ( "Green", None,                             # name, stock id
            "_Green", "<control><shift>G",             # label, accelerator
            "Grass", COLOR_GREEN ),                    # tooltip, value
          ( "Blue", None,                              # name, stock id
            "_Blue", "<control><shift>B",              # label, accelerator
            "Sky", COLOR_BLUE ),                       # tooltip, value
        )

        # GtkRadioActionEntry
        shape_entries = (
          ( "Square", None,                            # name, stock id
            "_Square", "<control><shift>S",            # label, accelerator
            "Square",  SHAPE_SQUARE ),                 # tooltip, value
          ( "Rectangle", None,                         # name, stock id
            "_Rectangle", "<control><shift>R",         # label, accelerator
            "Rectangle", SHAPE_RECTANGLE ),            # tooltip, value
          ( "Oval", None,                              # name, stock id
            "_Oval", "<control><shift>O",              # label, accelerator
            "Egg", SHAPE_OVAL ),                       # tooltip, value
        )

        # Create the menubar and toolbar
        action_group = gtk.ActionGroup("AppWindowActions")
        action_group.add_actions(entries)
        action_group.add_toggle_actions(toggle_entries)
        action_group.add_radio_actions(color_entries, COLOR_RED, self.activate_radio_action)
        action_group.add_radio_actions(shape_entries, SHAPE_OVAL, self.activate_radio_action)

        return action_group

    def show_dialog(self,message):
        dialog = gtk.MessageDialog(self, gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_INFO, gtk.BUTTONS_CLOSE,message)
        # Close dialog on user response
        dialog.connect("response", lambda d, r: d.destroy())
        dialog.show()

    def run_compiler(self, editor):
        editor.execute_script("returnEditorText(8080)")


    def open_file(self, editor):
        chooser = gtk.FileChooserDialog(title="Open File", action=gtk.FILE_CHOOSER_ACTION_OPEN,
                 buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        response = chooser.run()
        if response == gtk.RESPONSE_OK:
            openFiles.append(chooser.get_filename())
            with open (chooser.get_filename(), "r") as myfile:
                data=myfile.read()
                encoded = base64.b64encode(data)
                #chooser.get_filename().rfind("/")
                editor.execute_script("loadFile(\""+chooser.get_filename()+"\",\""+encoded+"\")")
        chooser.destroy()

    def activate_about(self, action):
        dialog = gtk.AboutDialog()
        dialog.set_name("PyGTK Demo")
        dialog.set_copyright("\302\251 Copyright 200x the PyGTK Team")
        dialog.set_website("http://www.pygtk.org./")
        ## Close dialog on user response
        dialog.connect ("response", lambda d, r: d.destroy())
        dialog.show()

    def get_text(self, parent, message, default=''):
        """
        Display a dialog with a text entry.
        Returns the text, or None if canceled.
        """
        d = gtk.MessageDialog(parent,
                              gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                              gtk.MESSAGE_QUESTION,
                              gtk.BUTTONS_OK_CANCEL,
                              message)
        entry = gtk.Entry()
        entry.set_text(default)
        entry.show()
        d.vbox.pack_end(entry)
        entry.connect('activate', lambda _: d.response(gtk.RESPONSE_OK))
        d.set_default_response(gtk.RESPONSE_OK)

        r = d.run()
        text = entry.get_text().decode('utf8')
        d.destroy()
        if r == gtk.RESPONSE_OK:
            return text
        else:
            return None

    def activate_action(self, action):
        if (self.webview.get_main_frame().get_uri() != editorWindowWebPage):
            self.webview.load_uri(editorWindowWebPage)
        global lastAction
        lastAction = action.get_name();
        if (lastAction == "New"):
            self.webview.execute_script("humane.log(\"Creating new document\")")
            global openFiles
            openFiles = []
            openFiles.append("programs/"+self.get_text(self, "What do you want to name your new file?",default="my awesome file")+".b4gl")
            self.set_title(openFiles[0])
        elif (lastAction == "Run"):
            self.run_compiler(self.webview)
            self.webview.execute_script("humane.log(\"Compiling {}\")".format(getFilename(openFiles[0])))
        elif(lastAction == "Open"):
            self.open_file(self.webview)
            self.set_title(openFiles[0])
        elif(lastAction == "Save"):
            self.run_compiler(self.webview)
            self.webview.execute_script("humane.log(\"Saved {}\")".format(getFilename(openFiles[0])))
        elif(lastAction == "Preferences"):
            print("Opening preferences")
            #self.preferencesDialog.get_widget("preferencesDialog").show()
            #preferencesWindow.connect("destroy", gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT )
            self.preferencesDialog.show()
            print("preferences open")
        else:
            self.webview.execute_script("humane.log(\"{}\");".format(action.get_name()))

    def activate_radio_action(self, action, current):
        active = current.get_active()
        value = current.get_current_value()

        if active:
            dialog = gtk.MessageDialog(self, gtk.DIALOG_DESTROY_WITH_PARENT,
                gtk.MESSAGE_INFO, gtk.BUTTONS_CLOSE,
                "You activated radio action: \"%s\" of type \"%s\".\nCurrent value: %d" %
                (current.get_name(), type(current), value))

            # Close dialog on user response
            dialog.connect("response", lambda d, r: d.destroy())
            dialog.show()

    def update_statusbar(self, buffer):
        # clear any previous message, underflow is allowed
        self.statusbar.pop(0)
        count = buffer.get_char_count()
        iter = buffer.get_iter_at_mark(buffer.get_insert())
        row = iter.get_line()
        col = iter.get_line_offset()
        self.statusbar.push(0,
        'Cursor at row %d column %d - %d chars in document' % (row, col, count))

    def update_resize_grip(self, widget, event):
        mask = gtk.gdk.WINDOW_STATE_MAXIMIZED | gtk.gdk.WINDOW_STATE_FULLSCREEN
        if (event.changed_mask & mask):
            self.statusbar.set_has_resize_grip(not (event.new_window_state & mask))

guiClass = None

def main():
    global guiClass
    guiClass = ApplicationMainWindowDemo()
    gtk.main()


def execute(command):
    cmd = command
    process = subprocess.Popen(shlex.split(command),stdout=subprocess.PIPE)
    out, err = process.communicate()
    exit_code = process.wait()
    return out

def getFilename(path):
    return ntpath.basename(path)

def doSave(message, path):
    print(message)
    fileText = base64.b64decode(message)
    with open (path, 'w') as myfile:
        myfile.write(fileText)

def passToCompiler(path):
    #placeholder
    #print("."+os.getcwd()+"/compiler/TINY")
    #print(path)
    output = execute(os.getcwd()+"/compiler/b4glC "+ path)
    guiClass.show_dialog(output)
    print(output)

def doWork( message):
    print(lastAction)
    if (lastAction=="Run"):
        doSave(message,openFiles[0])
        passToCompiler(openFiles[0])
    if (lastAction=="Save"):
        print("ok");
        doSave(message, openFiles[0])

def on_message(self, message):
    doWork(message)
    #print("openfiles = "+openFiles[0])
    #print("cwd = "+os.getcwd()+"/compiler/TINY")
    #print(execute(["./"+os.getcwd()+"/compiler/TINY",openFiles[0]]))



if __name__ == '__main__':
    t = threading.Thread(target=(WebSocketServer(8080, on_message).serve))
    t.daemon = True
    t.start()

    #t = threading.Thread(target=websocket.StartServer(on_message).serve)
    #t.daemon = True
    #t.start()

    main()
    ALIVE = False
