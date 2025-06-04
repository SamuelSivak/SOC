{
  "targets": [{
    "target_name": "neural_network",
    "sources": [ 
      "neural_network_wrapper.c",
      "../../src/Activation.c",
      "../../src/Data.c",
      "../../src/Evaluation.c",
      "../../src/Layer.c",
      "../../src/Loss.c",
      "../../src/Matrix.c",
      "../../src/Neuron.c",
      "../../src/NeuronNetwork.c",
      "../../src/Optimizer.c"
    ],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")",
      "../../include"
    ],
    "dependencies": [
      "<!(node -p \"require('node-addon-api').gyp\")"
    ],
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
    "libraries": ["-lm"]
  }]
}
