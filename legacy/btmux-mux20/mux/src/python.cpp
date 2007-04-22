/* 
 * $Id: python.cpp,v 1.0 2005/10/16 09:01:05 nick $
 * 
 * Author: Nicholas York <nicholas.york@gmail.com> 
 *
 * getPythonTraceback() based on code from 
 * Mathieu Fenniak <mathieu@stompstompstomp.com
 * http://stompstompstomp.com/weblog/technical/2004-03-29
 *
 */

#include "python.h"

PyObject *PyStdout(dbref executor);
extern class Python *python;

// these globals are used to capture stdout from python for function calls
char *functionoutput;
int functioncall = 0;

// this function by Mathieu Fenniak mathieu@stompstompstomp.com
// http://stompstompstomp.com/weblog/technical/2004-03-29
char *getPythonTraceback() {
    // Python equivalent:
    // import traceback, sys
    // return "".join(traceback.format_exception(sys.exc_type,
    //  sys.exc_value, sys.exc_traceback))

    PyObject *type, *value, *traceback;
    PyObject *tracebackModule;
    char *chrRetval;

    PyErr_Fetch(&type, &value, &traceback);
    tracebackModule = PyImport_ImportModule("traceback");
    if(tracebackModule != NULL) {
        PyObject *tbList, *emptyString, *strRetval;

        tbList = PyObject_CallMethod(
            tracebackModule,
            "format_exception",
            "OOO",
            type,
            value == NULL ? Py_None : value,
            traceback == NULL ? Py_None : traceback);

        emptyString = PyString_FromString("");
        strRetval = PyObject_CallMethod(emptyString, "join", "O", tbList);
        chrRetval = strdup(PyString_AsString(strRetval));

        Py_DECREF(tbList);
        Py_DECREF(emptyString);
        Py_DECREF(strRetval);
        Py_DECREF(tracebackModule);
    } else {
        chrRetval = strdup("Unable to import traceback module.");
    }

    Py_DECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    return chrRetval;
}

// add function to mux scheduler, and recall itself every 1s
void dispatch_python(void *pUnused, int iUnused) {
    python->Update();

    CLinearTimeAbsolute ltaNextTime;
    ltaNextTime.GetUTC();
    ltaNextTime += time_1s;
    scheduler.DeferTask(ltaNextTime, PRIORITY_SYSTEM+1, dispatch_python, 0, 0);
}
    
Python::Python() {
    Py_SetProgramName("netmux");
    Py_Initialize();                                        // set up the interpreter
    
    m_MainModule = PyImport_ImportModule("__main__");
    if(m_MainModule == NULL) {
        char *tb = getPythonTraceback();
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text(tb);
        ENDLOG;
    }

    m_SysModule = PyImport_ImportModule("sys");
    if(m_SysModule == NULL) {
        char *tb = getPythonTraceback();
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text(tb);
        ENDLOG;
    }
    PyObject *path = (m_SysModule ? PyDict_GetItemString(PyModule_GetDict(m_SysModule), "path") : NULL);
    if(path!=NULL) 
        PyList_Append(path, Py_BuildValue("s", "pymods"));
    else {
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text("Can't append to python path.");
        ENDLOG;
    }

    m_btModule = PyImport_ImportModule("bt");
    if(m_btModule == NULL) {
        char *tb = getPythonTraceback();
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text(tb);
        ENDLOG;
    }

    m_btwizModule = PyImport_ImportModule("btwiz");
    if(m_btwizModule == NULL) {
        char *tb = getPythonTraceback();
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text(tb);
        ENDLOG;
    }

    // start running Update() in 15 seconds to allow everything a chance to get going
    CLinearTimeAbsolute ltaNextTime;
    ltaNextTime.GetUTC();
    ltaNextTime += time_15s;
    scheduler.DeferTask(ltaNextTime, PRIORITY_SYSTEM+1, dispatch_python, 0, 0);

    m_Running = true;                                       // this will allow @python/python() to run
    STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
    log_text("Python Interpreter initialized!");
    ENDLOG;
}

Python::~Python() {
    Py_Finalize();                                          // shut down the python interpreter
    m_Running = false;                                      // toggle the flag so @python/python() are skipped
    STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
    log_text("Python Interpreter shutdown!");
    ENDLOG;
}

// this is called on database load
void Python::Load() {
    Run(0, "hook_load()");
    STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
    log_text("Loaded!");
    ENDLOG;
}

// called before every db dump
void Python::Save() {
    Run(0, "hook_save()");
    STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
    log_text("Saved!");
    ENDLOG;
}

// this is called every second
void Python::Update() {
    Run(0, "hook_update()");
}

void Python::CheckDicts(dbref executor) {

    // make sure the user has a global namespace in the map<dbref, PyObject *>
    DictMap::iterator globals = m_GlobalsMap.find(executor);
    if(globals == m_GlobalsMap.end()) 
        m_GlobalsMap[executor] = PyDict_New();

    // make sure the user has a local namespace in the map<dbref, PyObject *>
    DictMap::iterator locals = m_GlobalsMap.find(executor);
    if(locals == m_LocalsMap.end()) 
        m_LocalsMap[executor] = PyDict_New();

    // if the global namespace doesnt have a __builtins__ we need to load it
    if(PyDict_GetItemString(m_GlobalsMap[executor], "__builtins__") == NULL) {
        PyObject *builtinMod;
        if(Wizard(executor)) {          // if they're a wiz give them full control
            builtinMod = PyImport_ImportModule("__builtin__");
            PyDict_Update(m_GlobalsMap[executor], PyModule_GetDict(m_MainModule)); 
            PyDict_Update(m_GlobalsMap[executor], PyModule_GetDict(m_SysModule)); 
            PyDict_Update(m_GlobalsMap[executor], PyModule_GetDict(m_btModule)); 
            PyDict_Update(m_GlobalsMap[executor], PyModule_GetDict(m_btwizModule)); 
        } else {                        // otherwise give them a very restricted environment
            builtinMod = PyImport_ImportModule("__restricted_builtin__");
            PyDict_Update(m_LocalsMap[executor], PyModule_GetDict(m_btModule)); 
        }

        // includes either __builtin__ or __restricted_builtin__ in the namespace as __builtins__
        if(builtinMod == NULL || PyDict_SetItemString(m_GlobalsMap[executor], "__builtins__", builtinMod) != 0) {
            char *tb = getPythonTraceback();
            Py_DECREF(m_GlobalsMap[executor]);
            Py_XDECREF(m_GlobalsMap[executor]);
            notify(executor, "Can't add __builtin__, contact a Wizard.");
            raw_notify_html(executor, tb);
            return;
        }
        Py_DECREF(builtinMod);
    }
}

// this is used for @python, or for save/load/update above
void Python::Run(dbref executor, char *pstr) {

    if(!m_Running) {                                // if the python intepreter isn't running we can't do anything
        notify(executor, "Python not running.");
        return;
    }

    CheckDicts(executor);                           // make sure the executor has a global and local namespace

    PyObject *stdo;                                 // redirect stdout so that we can get the result
    if(executor) 
        stdo = PyStdout(executor);

    if(!m_CodeMap[executor]) {
        m_CodeMap[executor] = alloc_lbuf("python");
        m_CodeMap[executor][0] = '\0';
    }

    if(!m_CodeMap[executor]) {
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text("Allocation error!");
        ENDLOG;
        return;
    }

    int i, j;
    i = strlen(pstr);
    j = strlen(m_CodeMap[executor]);

    if(pstr[0] == '\n' && j == 0)
        return;

    if(j==0)
        m_CodeMap[executor][0] = '\0';

    strncat(m_CodeMap[executor], pstr, i);
    m_CodeMap[executor][i+j] = '\n';
    m_CodeMap[executor][i+j+1] = '\0';
    PyObject *src = Py_CompileString(m_CodeMap[executor], "<stdin>", Py_single_input);
    if(src!=NULL) {                                         // if the code compiles as is
        if(j==0 || m_CodeMap[executor][i+j-1] == '\n') {    // if we have pending input, wait for a blankline
            PyObject *dum = PyEval_EvalCode((PyCodeObject *)src, m_GlobalsMap[executor], m_LocalsMap[executor]);
            if( !dum ) {                                    // we got an error on our compiled code (like undefined variable)
                char *tb = getPythonTraceback();            // send the traceback data on failure
                notify(executor, tb);
            } 
            Py_XDECREF(dum);
            Py_XDECREF(src);
            free_lbuf(m_CodeMap[executor]);                 // we've run the command, so free up any used space
            m_CodeMap[executor] = NULL;
        } else {                                            // we've got a command that has compiled correctly, waiting on a newline
            notify(executor, "...");
        }
    } else {
        if(PyErr_ExceptionMatches(PyExc_SyntaxError)) {     // there was an error compiling, but it was a syntax error
            PyObject *exc, *val, *trb, *obj;
            PyErr_Fetch(&exc, &val, &trb);
            char *msg;                                      // pull out the message, see if it relates to EOF
            if(PyArg_ParseTuple(val, "sO", &msg, &obj) && !strcmp(msg, "unexpected EOF while parsing")) {
                Py_XDECREF(exc);
                Py_XDECREF(val);
                Py_XDECREF(trb);
                notify(executor, "...");                    // give the user an indication we're waiting on more input
            } else {
                PyErr_Restore(exc, val, trb);               // wasn't the error we were looking for so put everything back
                char *tb = getPythonTraceback();            // and handle normally
                notify(executor, tb);
                free_lbuf(m_CodeMap[executor]);
                m_CodeMap[executor] = NULL;
            }
        } else {
            char *tb = getPythonTraceback();                // code didnt compile, and it wasnt a syntax error
            notify(executor, tb);
            free_lbuf(m_CodeMap[executor]);
            m_CodeMap[executor] = NULL;
        } 
    } 

    if(executor)
        Py_DECREF(stdo);
}

// here is [python()]
FUNCTION(fun_python)
{
    if(!python->m_Running) {                        // if the python interpreter isn't running we can't do anything
        notify(executor, "Python not running.");
        return;
    }

    python->CheckDicts(executor);           // make sure that the executor has a global and local namespace
    functioncall = 1;                       // this tells the stdout object not to notify() the user, because
                                            // we want the output returned via function output
                                            
    char *argstr;

    PyObject *stdo;                         // redirect stdout so that we can get the result
    if(executor)                        
        stdo = PyStdout(executor);

    for(int i = 0; i < nfargs; i++) {       // set up so users can put one line per arg
        argstr = fargs[i];                  // such as: [python(import math,math.sqrt\(10\))]
                                            // the () inside have to be escaped to make it through the mux parser
        if(!argstr) {
            functioncall = 0;
            return;
        }

        while(*argstr && mux_isspace(*argstr))
            argstr++;
    
        if(!*argstr) {
            functioncall = 0;
            return;
        }

        functionoutput = "\0";              // make sure we don't get junk on error or if the command has no output
                                            // like 'import math'
        PyObject *resultObj = PyRun_String(argstr, Py_single_input, static_cast <PyObject *> (python->m_GlobalsMap[executor]), static_cast <PyObject *> (python->m_LocalsMap[executor]));

        if( !resultObj ) {
            char *tb = getPythonTraceback();    // get the traceback so we know why it failed
            notify(executor, tb);               // send to executor
        } else
            safe_str(functionoutput, buff, bufc);      // copy the output from stdout to buff so its returned to the user
    }

    if(executor)
        Py_DECREF(stdo);

    functioncall = 0;                       // resets this for the next @python or python() call
}

// We're going to create a new python object, with a write method
// and connect python's stdout/stderr there
//
// if the call is via @python or , then it will notify() the executor
//
// if the call is via function, it captures the output for return in buff

typedef struct {
    PyObject_HEAD;
    dbref sodbref;
} muxstdout;

PyObject *muxstdout_write(muxstdout *so, PyObject *args) {

    char *str;
    if(!PyArg_ParseTuple(args, "s:write", &str)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // notify() adds its own newline, so strip it if there is one
    if(str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';

    if(*str) {
        if(functioncall)                  // if this is called via python()
            functionoutput = str;         // using a global variable for now, this is returned from the function call as buff
        else                              // if this is called via @python
            notify(so->sodbref, str);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyMethodDef muxstdout_methods[] = {
    {"write", (PyCFunction)muxstdout_write, METH_VARARGS},      // stdout.write() will go here
    {NULL, NULL}
};

PyObject *muxstdout_getattr(PyObject *so, char *name) {

    return Py_FindMethod(muxstdout_methods, so, name);
}

void muxstdout_dealloc(muxstdout *so) {

    PyMem_DEL(so);
}

PyTypeObject muxstdout_Type = {
    PyObject_HEAD_INIT(&PyType_Type) 
    0,                                  /* ob_size */
    "muxstdout",                       /* tp_name */
    sizeof(muxstdout),                 /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)muxstdout_dealloc,     /* tp_dealloc */
    0,                                  /* tp_print */
    (getattrfunc)muxstdout_getattr,    /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "tinymux stdout",                   /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,                                  /* tp_init */
    0,                                  /* tp_alloc */
    0,                                  /* tp_new */
};

PyObject *makestdout(dbref sodbref) {

    muxstdout *newstdout = PyObject_NEW(muxstdout, &muxstdout_Type);

    if(newstdout == NULL)
        return NULL;

    newstdout->sodbref = sodbref;
    return (PyObject *)newstdout;
}

PyObject *PyStdout(dbref executor) {

    PyObject *stdo = makestdout(executor);

    if (PySys_SetObject("stdout", stdo) == -1) {
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text("Can't set stdout.");
        ENDLOG;
        return NULL;
    }

    if (PySys_SetObject("stderr", stdo) == -1) {
        STARTLOG(LOG_WIZARD, "WIZ", "PYTHON");
        log_text("Can't set stderr.");
        ENDLOG;
        return NULL;
    }

    return stdo;
}
