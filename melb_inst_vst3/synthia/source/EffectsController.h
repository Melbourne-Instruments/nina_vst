/*
 *  mdaBaseController.h
 *  mda-vst3
 *
 *  Created by Arne Scheffler on 6/14/08.
 *
 *  mda VST Plug-ins
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: The above copyright
 * notice and this permission notice shall be included in all copies or
 * substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include "common.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <stdio.h>

namespace Steinberg {
namespace Vst {
namespace Nina {

//-----------------------------------------------------------------------------
class EffectsController : public EditController, public IMidiMapping {
  public:
    // --- EditController Overrides
    EffectsController(){};
    ~EffectsController(){};
    tresult PLUGIN_API initialize(FUnknown *context);
    tresult PLUGIN_API terminate();

    // --- serialize-read from file
    tresult PLUGIN_API setComponentState(IBStream *fileStream);

    // --- IMidiMapping
    virtual tresult PLUGIN_API getMidiControllerAssignment(
        int32 busIndex, int16 channel, CtrlNumber midiControllerNumber,
        ParamID &id /*out*/);

    // --- oridinarily not needed; see documentation on Automation for using these
    virtual ParamValue PLUGIN_API
    normalizedParamToPlain(ParamID id, ParamValue valueNormalized);
    virtual ParamValue PLUGIN_API plainParamToNormalized(ParamID id,
        ParamValue plainValue);

    // --- Our COM Creating Method
    static FUnknown *createInstance(void *) {
        return (IEditController *)new EffectsController;
    }

    // override this function to mask off the layer state bits of the param ID, this is so we return the actual parameter
    virtual Parameter *getParameterObject(ParamID tag) override {
        return parameters.getParameter(mask_param_id(tag));
    }

    // --- our globally unique ID value
    static FUID uid;

    // --- helper function for serialization
    tresult PLUGIN_API setParamNormalizedFromFile(ParamID tag, ParamValue value);

    void generateMatrixParams();
    // --- define the controller and interface
    OBJ_METHODS(EffectsController, EditController)
    DEFINE_INTERFACES
    DEF_INTERFACE(IMidiMapping)
    END_DEFINE_INTERFACES(EditController)
    REFCOUNT_METHODS(EditController)
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
