#include <node_api.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/NeuronNetwork.h"
#include "../../include/Data.h"

static NeuralNetwork* nn = NULL;

napi_value Init(napi_env env, napi_callback_info info){
    napi_status status;
    size_t argc = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if(status != napi_ok) return NULL;

    char modelPath[256];
    size_t str_len;
    status = napi_get_value_string_utf8(env, args[0], modelPath, 256, &str_len);
    if(status != napi_ok) return NULL;

    if(nn != NULL){
        network_free(nn);
    }
    nn = network_load(modelPath);

    napi_value result;
    napi_get_boolean(env, nn != NULL, &result);
    return result;
}

napi_value Predict(napi_env env, napi_callback_info info){
    napi_status status;
    size_t argc = 1;
    napi_value args[1];
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if(status != napi_ok) return NULL;

    bool is_array;
    status = napi_is_array(env, args[0], &is_array);
    if(status != napi_ok || !is_array) return NULL;

    uint32_t length;
    status = napi_get_array_length(env, args[0], &length);
    if(status != napi_ok || length != 784) return NULL;  // 28x28 = 784

    float* input = malloc(784 * sizeof(float));
    for(uint32_t i = 0; i < length; i++){
        napi_value element;
        status = napi_get_element(env, args[0], i, &element);
        if(status != napi_ok){
            free(input);
            return NULL;
        }
        double value;
        status = napi_get_value_double(env, element, &value);
        if(status != napi_ok){
            free(input);
            return NULL;
        }
        input[i] = (float)value;
    }

    network_forward(nn, input);
    float* predictions = nn->output_data;
    free(input);

    napi_value result;
    status = napi_create_array_with_length(env, 10, &result);
    if(status != napi_ok) return NULL;

    for(int i = 0; i < 10; i++){
        napi_value pred;
        status = napi_create_double(env, predictions[i], &pred);
        if(status != napi_ok) return NULL;
        status = napi_set_element(env, result, i, pred);
        if(status != napi_ok) return NULL;
    }

    return result;
}

napi_value GetModelInfo(napi_env env, napi_callback_info info){
    napi_status status;
    napi_value result;
    status = napi_create_object(env, &result);
    if(status != napi_ok) return NULL;

    if(nn == NULL){
        napi_value loaded;
        status = napi_get_boolean(env, false, &loaded);
        if(status != napi_ok) return NULL;
        status = napi_set_named_property(env, result, "loaded", loaded);
        if(status != napi_ok) return NULL;
        return result;
    }

    napi_value loaded;
    status = napi_get_boolean(env, true, &loaded);
    if(status != napi_ok) return NULL;
    status = napi_set_named_property(env, result, "loaded", loaded);
    if(status != napi_ok) return NULL;

    napi_value layers;
    status = napi_create_uint32(env, nn->num_layers, &layers);
    if(status != napi_ok) return NULL;
    status = napi_set_named_property(env, result, "numLayers", layers);
    if(status != napi_ok) return NULL;

    return result;
}

napi_value Cleanup(napi_env env, napi_callback_info info){
    if(nn != NULL){
        network_free(nn);
        nn = NULL;
    }

    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init_Addon(napi_env env, napi_value exports){
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("init", Init),
        DECLARE_NAPI_METHOD("predict", Predict),
        DECLARE_NAPI_METHOD("getModelInfo", GetModelInfo),
        DECLARE_NAPI_METHOD("cleanup", Cleanup)
    };
    status = napi_define_properties(env, exports, 4, desc);
    if(status != napi_ok) return NULL;
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init_Addon) 