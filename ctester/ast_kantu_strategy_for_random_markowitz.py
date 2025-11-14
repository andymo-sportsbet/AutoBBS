#!/usr/bin/python
import subprocess
import sys
import linecache
from tempfile import mkstemp
from shutil import copy
from os import remove
import argparse
import os

def main():
    #first diagnostic

    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifier')
    parser.add_argument('-s', '--symbol')
    parser.add_argument('-tf', '--timeframe')
    parser.add_argument('-sp', '--spread')
    parser.add_argument('-d', '--digits')
    parser.add_argument('-ty', '--type')
    parser.add_argument('-ms', '--minStop')
    args = parser.parse_args()

    kantuID = args.identifier
    symbol = args.symbol
    timeframe = args.timeframe
    spread = args.spread
    digits = args.digits
    simulationType = args.type
    minStop = args.minStop

    if kantuID == None:
        print "Kantu ID missing (id)"
        sys.exit()

    if symbol == None:
        print "symbol missing (s)"
        sys.exit()

    if timeframe == None:
        print "timeframe missing (tf)"
        sys.exit()

    if spread == None:
        print "spread missing (sp)"

    if digits == None:
        print "digits missing (d)"
        sys.exit()

    if simulationType == None:
        print "requested type missing (ty)"
        sys.exit()

    if minStop == None:
        print "minimum stop missing (ms)"
        sys.exit()

    filePath = os.path.join(os.getcwd(), 'sets', 'kantu_test.set')

    with open(filePath, "w") as text_file:
        text_file.write("""COMMENTS=Click "Modify expert" to view comments inside the source code.
SECTION_1=################# General Settings #################
FRAMEWORK_CONFIG=./experts/config/AsirikuyConfig.xml
USE_ORDER_WRAPPER=0
ENABLE_SCREENSHOTS=0
HISTORIC_DATA_ID=0
MAX_SLIPPAGE_PIPS=2.00000000
UI_FONT_SIZE=12
UI_SYSTEM_INFO_COLOR=16777215
UI_PROFIT_INFO_COLOR=65535
UI_TRADE_INFO_COLOR=42495
UI_ERROR_INFO_COLOR=255
UI_CUSTOM_INFO_COLOR=15453831
CUSTOM_ORDER_COMMENT=
SECTION_2=############## Common Strategy Settings ##############
OPERATIONAL_MODE=1
RUN_EVERY_TICK=0
STRATEGY_INSTANCE_ID=1
STRATEGY_TIMEFRAME=
ACCOUNT_RISK_PERCENT=0.10
MAX_DRAWDOWN_PERCENT=100.00000000
MAX_SPREAD_PIPS=100.00000000
INSTANCE_MANAGEMENT=0
WFO_PSET_MANAGEMENT=0
ANALYSIS_WINDOW_SIZE=0
PARAMETER_SET_POOL=0.00000000
DISABLE_COMPOUNDING=0
USE_INSTANCE_BALANCE=0
INIT_INSTANCE_BALANCE=0.00000000
MAX_OPEN_ORDERS=2
TIMED_EXIT_BARS=0
SL_ATR_MULTIPLIER=0.0
TP_ATR_MULTIPLIER=0.00000000
ATR_AVERAGING_PERIOD=20
SECTION_3=############## Additional Strategy Settings ##############
SELECTED_STRATEGY_ID=-1.0
VOLATILITY_CALCULATION_MODE=0
TL_ATR_MULTIPLIER = 0.0
USE_AFMTL = 0.0
SELECT_KANTU_SYSTEM_FILE = 0
DSL_EXIT_TYPE=0
""")

    filePath = os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config')

    with open(filePath, "w") as text_file:
        text_file.write("""[misc]
logSeverity = 0 			; 0 = Emergency, 1 = Alert, 2 = Critical, 3 = Error, 4 = Warning, 5 = Notice, 6 = Info, 7 = Debug. The selected severity level plus all lower levels will be logged

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 
stopOutPercent = 1
minimumStop =
spread =
minLotSize = 0.01

[strategy]
setFile = kantu_test.set
pair =
strategyID = 18
passedTimeFrame = 60
fromDate = 01/01/86			; dd/mm/yy
toDate   = 26/04/15 		; dd/mm/yy
rateRequirements =
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 0
optimizationType = 1		; 0 = Brute Force, 1 = Genetic
numCores = 2				; Only for OpenMP parallel optimization

;Advanced parameters 
;for genetic optimization

optimizationGoal = 0		; 0 = profit, 1 = Maximum DD, 2 = Maximum DD in Length, 3 = PF, 4 = R2, 5 = Ulcer Index
population = 25
maxGenerations = 10000			; 0 to not limit
stopIfConverged = 1
discardAssymetricSets = 1
minTradesAYear = 20

crossoverProbability = 0.9
mutationProbability = 0.2
migrationProbability = 0.0
evolutionaryMode = 0 		;0 = Darwin, 1 = Lamarck Parents, 2 = Lamarck Children, 3 = Lamarck All, 4 = Baldwin Parents, 8 = Baldwin Children 12 = Baldwin All
elitismMode = 1 			;0 = Unknown, 1 = Parents survive, 2 = One parent survives, 3 = Parents die, 4 = Rescore Parents
mutationMode = 0			;0 = Single point drift, 1 = Single point randomize, 2 = Multipoint, 3 = All point
crossoverMode = 0 			;0 = Single points, 1 = Double points, 3 = Mean, 4 = Mixing, 5 = Allele mixing

""")

    filePath = os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config')

    with open(filePath, "w") as text_file:
        text_file.write("""[misc]
logSeverity = 0 			; 0 = Emergency, 1 = Alert, 2 = Critical, 3 = Error, 4 = Warning, 5 = Notice, 6 = Info, 7 = Debug. The selected severity level plus all lower levels will be logged

[account]
currency = USD
brokerName = Alpari (UK) Historic Data
refBrokerName = Alpari (UK) Historic Data
balance = 100000
leverage = 200
contractSize = 100000
digits = 
stopOutPercent = 1
minimumStop =
spread =
minLotSize = 0.01

[strategy]
setFile = kantu_test.set
pair =
strategyID = 18
passedTimeFrame = 60
fromDate = 01/01/86			; dd/mm/yy
toDate   = 26/04/15 		; dd/mm/yy
rateRequirements =
symbolRequirements = D
timeframeRequirements = 0

[optimization]
optimize = 0
optimizationType = 1		; 0 = Brute Force, 1 = Genetic
numCores = 2				; Only for OpenMP parallel optimization

;Advanced parameters 
;for genetic optimization

optimizationGoal = 0		; 0 = profit, 1 = Maximum DD, 2 = Maximum DD in Length, 3 = PF, 4 = R2, 5 = Ulcer Index
population = 25
maxGenerations = 10000			; 0 to not limit
stopIfConverged = 1
discardAssymetricSets = 1
minTradesAYear = 20

crossoverProbability = 0.9
mutationProbability = 0.2
migrationProbability = 0.0
evolutionaryMode = 0 		;0 = Darwin, 1 = Lamarck Parents, 2 = Lamarck Children, 3 = Lamarck All, 4 = Baldwin Parents, 8 = Baldwin Children 12 = Baldwin All
elitismMode = 1 			;0 = Unknown, 1 = Parents survive, 2 = One parent survives, 3 = Parents die, 4 = Rescore Parents
mutationMode = 0			;0 = Single point drift, 1 = Single point randomize, 2 = Multipoint, 3 = All point
crossoverMode = 0 			;0 = Single points, 1 = Double points, 3 = Mean, 4 = Mixing, 5 = Allele mixing

""")

    if simulationType == "AUK" or simulationType == "ALL" or simulationType == "AUK_NC" or simulationType == "AUK_C":
        print "modifying AUK config to match symbol"
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "pair =", "pair =" + symbol)
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "digits =", "digits =" + digits)
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "spread =", "spread =" + spread)
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "minimumStop =", "minimumStop =" + minStop)

        if int(timeframe) == 1440:
            replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "rateRequirements =", "rateRequirements =" + str(200))
        else:
            replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_auk.config'), "rateRequirements =", "rateRequirements =" + str((1440/int(timeframe))*40))

    if simulationType == "1987" or simulationType == "ALL" or simulationType == "1987_NC" or simulationType == "1987_C":
        print "modifying 1987_config to match symbol"
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "pair =", "pair =" + symbol + "1987")
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "digits =", "digits =" + digits)
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "spread =", "spread =" + spread)
        replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "minimumStop =", "minimumStop =" + minStop)

        if int(timeframe) == 1440:
            replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "rateRequirements =", "rateRequirements =" + str(200))
        else:
            replace(os.path.join(os.getcwd(), 'config', 'ast_kantu_1987.config'), "rateRequirements =", "rateRequirements =" + str((1440/int(timeframe))*40))

    
    replace(os.path.join(os.getcwd(), 'sets', 'kantu_test.set'), "SELECT_KANTU_SYSTEM_FILE = 0", "SELECT_KANTU_SYSTEM_FILE = " + kantuID)
    replace(os.path.join(os.getcwd(), 'sets', 'kantu_test.set'), "STRATEGY_TIMEFRAME=", "STRATEGY_TIMEFRAME=" + timeframe)

    if simulationType == "1987" or simulationType == "ALL" or simulationType == "1987_C":
        print "Running 1987 compounding test"
        subprocess.check_output("python asirikuy_strategy_tester.py -ot sys_" +kantuID+"_compounding_1987 -c ./config/ast_kantu_1987.config", shell=True)

    if simulationType == "AUK" or simulationType == "ALL" or simulationType == "AUK_C": 
        print "Running AUK compounding test"
        subprocess.check_output("python asirikuy_strategy_tester.py -ot sys_" +kantuID+"_compounding -c ./config/ast_kantu_auk.config", shell=True)

    replace(os.path.join(os.getcwd(), 'sets', 'kantu_test.set'), "DISABLE_COMPOUNDING=0", "DISABLE_COMPOUNDING=1")

    if simulationType == "1987" or simulationType == "ALL" or simulationType == "1987_NC":
        print "Running 1987 non-compounding test"
        subprocess.check_output("python asirikuy_strategy_tester.py -ot sys_" +kantuID+"_1987 -c ./config/ast_kantu_1987.config", shell=True)

    if simulationType == "AUK" or simulationType == "ALL" or simulationType == "AUK_NC":
        print "Running AUK non-compounding test"
        subprocess.check_output("python asirikuy_strategy_tester.py -ot sys_" +kantuID+" -c ./config/ast_kantu_auk.config", shell=True)

    print "Finished Kantu strategy tests :o)"

def replace(source_file_path, pattern, substring):
    fh, target_file_path = mkstemp()
    f1 = open(target_file_path, 'w')
    f2 = open(source_file_path, 'r')
    with f1 as target_file:
        with f2 as source_file:
            for line in source_file:
                target_file.write(line.replace(pattern, substring))
    f2.close()
    f1.close()
    remove(source_file_path)
    copy(target_file_path, source_file_path)

            
##################################
###           MAIN           ####
##################################

if __name__ == "__main__": main()
