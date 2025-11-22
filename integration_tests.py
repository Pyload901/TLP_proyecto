import os
import subprocess
import sys
import re

# Paths
ROOT_DIR = os.path.abspath(".")
LANGUAGE_DIR = os.path.join(ROOT_DIR, "language")
VM_TEST_DIR = os.path.join(ROOT_DIR, "vm/test")
PARSER_EXE = os.path.join(LANGUAGE_DIR, "parser")
VM_RUNNER_EXE = os.path.join(VM_TEST_DIR, "vm_runner")
TEST_SRC = os.path.join(LANGUAGE_DIR, "test.src")
VM_CODE = os.path.join(LANGUAGE_DIR, "program.vmcode")

def run_command(cmd, cwd=None):
    try:
        result = subprocess.run(cmd, cwd=cwd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {' '.join(cmd)}")
        print("STDOUT:", e.stdout)
        print("STDERR:", e.stderr)
        raise

def build_tools():
    print("Building compiler...")
    run_command(["make"], cwd=LANGUAGE_DIR)
    print("Building VM runner...")
    run_command(["make"], cwd=VM_TEST_DIR)

def parse_registers(output):
    # Look for "Regs: R0=..., R1=..."
    match = re.search(r"Regs: (.*)", output)
    if not match:
        return {}
    
    regs_str = match.group(1)
    regs = {}
    for part in regs_str.split(", "):
        key, val = part.split("=")
        regs[key] = int(val)
    return regs

def run_test(name, source_code, expected_regs):
    print(f"Running test: {name}")
    
    # Write source code
    with open(TEST_SRC, "w") as f:
        f.write(source_code)
    
    # Compile
    try:
        run_command([PARSER_EXE], cwd=LANGUAGE_DIR)
    except Exception:
        print(f"FAIL: Compilation failed for {name}")
        return False

    # Run VM
    try:
        output = run_command([VM_RUNNER_EXE, VM_CODE], cwd=VM_TEST_DIR)
    except Exception:
        print(f"FAIL: VM execution failed for {name}")
        return False

    # Check output
    actual_regs = parse_registers(output)
    
    all_ok = True
    for reg, expected_val in expected_regs.items():
        if reg not in actual_regs:
            print(f"FAIL: {name} - Register {reg} not found in output")
            all_ok = False
        elif actual_regs[reg] != expected_val:
            print(f"FAIL: {name} - {reg} = {actual_regs[reg]} (expected {expected_val})")
            all_ok = False
    
    if all_ok:
        print(f"PASS: {name}")
        return True
    else:
        print(f"Actual Regs: {actual_regs}")
        return False

def main():
    try:
        build_tools()
    except Exception:
        print("Failed to build tools. Exiting.")
        sys.exit(1)

    tests = [
        {
            "name": "Simple Assignment",
            "source": """
start
  int a = 42;
end
""",
            "expected_regs": {"R1": 42} # Assuming 'a' is R1
        },
        {
            "name": "Addition",
            "source": """
start
  int a = 10;
  int b = 20;
  a = a + b;
end
""",
            "expected_regs": {"R1": 30} # a = 30
        },
        {
            "name": "Loop",
            "source": """
start
  int x = 0;
  for (int i = 0; i < 5; i = i + 1) start
    x = x + 1;
  end
end
""",
            "expected_regs": {"R1": 5} # x = 5
        }
    ]
    
    passed = 0
    for test in tests:
        if run_test(test["name"], test["source"], test["expected_regs"]):
            passed += 1
    
    print(f"\nSummary: {passed}/{len(tests)} tests passed.")

if __name__ == "__main__":
    main()
