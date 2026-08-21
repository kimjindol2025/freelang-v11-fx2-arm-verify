const assert = require("assert");
const fs = require("fs");
const os = require("os");
const path = require("path");
const cp = require("child_process");
const { afjDbResult, afjDbStep } = require("../../../freelang-v11/scripts/ai-project");

const dbRoot = path.resolve(__dirname, "../../../afl-db/v2-afj");
const cli = path.join(dbRoot, "src/cli.js");

const invalidRoot = afjDbResult(__dirname, { database: { type: "afj-db", root: "/tmp/fl-afj-db-does-not-exist" } }, false);
assert.strictEqual(invalidRoot.status, "FAIL");
assert.strictEqual(invalidRoot.error_code, "FL_AFJ_DB_CONNECT_FAILED");
assert.strictEqual(invalidRoot.cleanup, "PASS");

function filesUnder(dir) {
  const result = [];
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const file = path.join(dir, entry.name);
    if (entry.isDirectory()) result.push(...filesUnder(file));
    else result.push(file);
  }
  return result;
}
const namespace = fs.mkdtempSync(path.join(os.tmpdir(), "fl-afj-db-failure-"));
const db = path.join(namespace, "db");
try {
  const append = cp.spawnSync(process.execPath, [cli, "append", db, "failure", JSON.stringify({ kind: "fl_verify_failure" }), "1"], { encoding: "utf8" });
  assert.strictEqual(append.status, 0);
  const log = filesUnder(db).find(file => /event|log|ndjson|jsonl/i.test(path.basename(file)));
  assert.ok(log, "AFJ DB append log was not found");
  fs.appendFileSync(log, "{corrupt-event}\n");
  const corrupted = afjDbStep(cli, dbRoot, ["verify", db], "afj_db_verify", data => Boolean(data && data.ok === true), "FL_AFJ_DB_VERIFY_FAILED", "inspect AFJ DB hash-chain verification output");
  assert.strictEqual(corrupted.status, "FAIL");
  assert.strictEqual(corrupted.error_code, "FL_AFJ_DB_VERIFY_FAILED");
  console.log(JSON.stringify({
    invalid_endpoint: { status: invalidRoot.status, error_code: invalidRoot.error_code, cleanup: invalidRoot.cleanup },
    corrupted_log: { status: corrupted.status, error_code: corrupted.error_code, exit_code: corrupted.exit_code }
  }, null, 2));
} finally {
  fs.rmSync(namespace, { recursive: true, force: true });
}
