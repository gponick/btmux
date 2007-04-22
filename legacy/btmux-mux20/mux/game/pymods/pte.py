import time
import threading

class MyThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        for i in range(15):
            print 'printed from MyThread...'
            time.sleep(1)

def createThread():
    print 'Create and run MyThread'
    background = MyThread()
    background.start()
    print 'Main thread continues to run in the foreground.'
    for i in range(10):
        print 'printed from the Main thread.'
        time.sleep(1)
    print 'Main thread joins MyThread and waits until it is done...'
    background.join()
    print 'The program completed gracefully'
