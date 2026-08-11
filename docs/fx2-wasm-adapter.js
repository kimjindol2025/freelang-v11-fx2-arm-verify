(function (global) {
  function createFx2Wasm(Module) {
    var ready = new Promise(function (resolve) {
      var previous = Module.onRuntimeInitialized;
      Module.onRuntimeInitialized = function () {
        if (previous) previous();
        resolve(api);
      };
    });

    var api = {
      ready: ready,
      smoke: function () {
        return Module.ccall('fx2_wasm_smoke', 'number', [], []) === 1;
      },
      onClick: function (selector, handler) {
        var node = document.querySelector(selector);
        if (!node) throw new Error('missing DOM node: ' + selector);
        node.addEventListener('click', handler);
        return function () { node.removeEventListener('click', handler); };
      },
      onInput: function (selector, handler) {
        var node = document.querySelector(selector);
        if (!node) throw new Error('missing DOM node: ' + selector);
        var listener = function (event) { handler(event.target.value, event); };
        node.addEventListener('input', listener);
        return function () { node.removeEventListener('input', listener); };
      }
    };
    return api;
  }

  global.createFx2Wasm = createFx2Wasm;
})(window);
