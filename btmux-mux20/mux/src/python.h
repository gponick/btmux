#ifndef PYTHON_H
#define PYTHON_H

#include <map>
#undef _POSIX_C_SOURCE
#include "Python.h"

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "externs.h"
#include "flags.h"
#include "functions.h"

typedef std::map<dbref, PyObject *> DictMap;
typedef std::map<dbref, char *> CodeMap;

class Python {
    public:
        bool m_Running;
        DictMap m_GlobalsMap;                   // global namespace per user
        DictMap m_LocalsMap;                    // local namespace per user
        CodeMap m_CodeMap;                      // incomplete multi-line code

        PyObject *m_MainModule;                 // only load modules once
        PyObject *m_SysModule;                  // keep these around to add to
        PyObject *m_btModule;                   // everyones namespaces
        PyObject *m_btwizModule;

        Python();
        ~Python();

        void Load();                            // on game startup
        void Save();                            // before every db save
        void Update();                          // called once per second

        void Run(dbref executor, char *pstr);   // executes a python command
    
        void CheckDicts(dbref executor);
};

FUNCTION(fun_python);
#endif
