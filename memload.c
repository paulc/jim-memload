
#include <jim.h>
#include "MemoryModule.h"

static void JimFreeLoadHandles(Jim_Interp *interp, void *data);

static void JimFreeOneLoadHandle(void *handle) {
    MemoryFreeLibrary(handle);
}

static void JimFreeLoadHandles(Jim_Interp *interp, void *data) {
    Jim_Stack *handles = data;

    if (handles) {
        Jim_FreeStackElements(handles, JimFreeOneLoadHandle);
        Jim_Free(handles);
    }
}

static int MemLoad_Cmd(Jim_Interp *interp, int argc, Jim_Obj *const argv[]) {

    Jim_Obj *name = argv[1];
    Jim_Obj *data = argv[2];

    HMEMORYMODULE handle;
    char initsym[80];
    typedef int jim_module_init_func_type(Jim_Interp *);
    jim_module_init_func_type *onload;

    if (argc != 3) {
        Jim_WrongNumArgs(interp,1,argv,"<name> <data>");
        return JIM_ERR;
    }

    if ((handle = MemoryLoadLibrary(data->bytes)) == NULL) {
        Jim_SetResultFormatted(interp,"Error loading extension: %s",Jim_String(name));
        return JIM_ERR;
    }

    snprintf(initsym, sizeof(initsym), "Jim_%sInit", Jim_String(name));

    if ((onload = (jim_module_init_func_type *)MemoryGetProcAddress(handle,initsym)) == NULL) {
        Jim_SetResultFormatted(interp,"No %s symbol found (%s)",initsym,Jim_String(name));
        return JIM_ERR;
    } else if (onload(interp) != JIM_ERR) {
        /* Add this handle to the stack of handles to be freed */
        Jim_Stack *loadHandles = Jim_GetAssocData(interp, "load::handles");
        if (loadHandles == NULL) {
            loadHandles = Jim_Alloc(sizeof(*loadHandles));
            Jim_InitStack(loadHandles);
            Jim_SetAssocData(interp, "load::handles", JimFreeLoadHandles, loadHandles);
        }
        Jim_StackPush(loadHandles, handle);

        // Jim_SetEmptyResult(interp);
        Jim_SetResultString(interp,initsym,-1);

        return JIM_OK;
    }
}

int Jim_memloadInit(Jim_Interp *interp) {
    Jim_CreateCommand(interp, "memload", MemLoad_Cmd, NULL, NULL);
    return JIM_OK;
}
