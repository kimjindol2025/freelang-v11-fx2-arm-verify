const assert = require("assert");
const path = require("path");
const { apiResult } = require("../../../freelang-v11/scripts/ai-project");

const root = __dirname;
const base = {
  backend: {
    type: "freelang-afj",
    start_command: "node ../../../freelang-v11/bootstrap.js run app.fl",
    health: "/health"
  },
  database: { type: "afj-db", root: "../../../afl-db/v2-afj" },
  api: { required: true, smoke: {} }
};

const badHealth = apiResult(root, {
  ...base,
  backend: { ...base.backend, health: "/invalid-health" }
});
assert.strictEqual(badHealth.status, "FAIL");
assert.strictEqual(badHealth.error_code, "FL_API_HEALTH_FAILED");
assert.strictEqual(badHealth.required, true);

const badStart = apiResult(root, {
  ...base,
  backend: { ...base.backend, start_command: "./missing-freelang-afj-backend" }
});
assert.strictEqual(badStart.status, "FAIL");
assert.strictEqual(badStart.error_code, "FL_AFJ_BACKEND_START_FAILED");
assert.strictEqual(badStart.required, true);

const badValue = apiResult(root, {
  ...base,
  api: { required: true, smoke: { expected: { phone: "01011111111" } } }
});
assert.strictEqual(badValue.status, "FAIL");
assert.strictEqual(badValue.error_code, "FL_API_VALUE_MISMATCH");
assert.strictEqual(badValue.required, true);

console.log(JSON.stringify({
  invalid_health: { status: badHealth.status, error_code: badHealth.error_code, can_claim_done: false },
  invalid_start: { status: badStart.status, error_code: badStart.error_code, can_claim_done: false },
  wrong_expected_value: { status: badValue.status, error_code: badValue.error_code, can_claim_done: false }
}, null, 2));
