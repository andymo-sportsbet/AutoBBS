@title = runNTS.bat

echo start Peso_MACD_GBPJPY-1H_860006 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Peso_MACD_GBPJPY-1H_860006.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_860006.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_860006.csv > ./log/runNTS.log
echo Finish Peso_MACD_GBPJPY-1H_860006 backtesting > ./log/runNTS.log

echo start Barnacles_GBPJPY-15M_850001 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Barnacles_GBPJPY-15M_850001.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_850001.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_850001.csv > ./log/runNTS.log
echo Finish Barnacles_GBPJPY-15M_850001 backtesting > ./log/runNTS.log

echo start Shellington_GBPJPY-1H_841005 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Shellington_GBPJPY-1H_841005.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_841005.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_841005.csv > ./log/runNTS.log
echo Finish Shellington_GBPJPY-1H_841005 backtesting > ./log/runNTS.log

echo start Shellington_XAUUSD-1H_842001 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Shellington_XAUUSD-1H_842001.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_842001.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_842001.csv > ./log/runNTS.log
echo Finish Shellington_XAUUSD-1H_842001 backtesting > ./log/runNTS.log

echo start Kwazii_XAUUSD-15M_840005 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Kwazii_XAUUSD-15M_840005.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_840005.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_840005.csv > ./log/runNTS.log
echo Finish Kwazii_XAUUSD-15M_840005 backtesting > ./log/runNTS.log

echo start Inkling_XAUUSD-5M_860001 backtesting > ./log/runNTS.log
python asirikuy_strategy_tester.py -c  ./config/Inkling_XAUUSD-5M_860001.config > ./log/runNTS.log
move /Y results.txt ./Batch/results_860001.txt > ./log/runNTS.log
move /Y allStatistics.csv ./Batch/allStatistics_860001.csv > ./log/runNTS.log
echo Finish Inkling_XAUUSD-5M_860001 backtesting > ./log/runNTS.log

