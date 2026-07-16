#!/usr/bin/env python3

import sys

def Usage():
    usageStr = """
    1. Build & Run Debug
    2. Build & Run Release
    3. Rebuild & Run Debug
    4. Rebuild & Run Release
    5. Build
    6. Rebuild
    7. Clean
    """
    print(usageStr)

def Build():
    return

def Clean():
    return

def Run(version):
    return

if __name__ == "__main__":
    numArgs = len(sys.argv)

    if numArgs == 1:
        Usage()
        exit(0)

    oper = sys.argv[1]

    if oper == 1:
        Build()
        Run("debug")

    elif oper == 2:
        Build()
        Run("release")

    elif oper == 3:
        Clean()
        Build()
        Run("debug")
        
    elif oper == 4:
        Clean()
        Build()
        Run("release")

    elif oper == 5:
        Build()
        
    elif oper == 6:
        Clean()
        Build()

    elif oper == 7:
        Clean()

    else:
        print("Invalid Operation", oper)

        
        



    
