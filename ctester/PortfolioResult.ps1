$Result_840005 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\840005.txt'
$Result_840007 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\840007.txt'
$Result_841005 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\841005.txt'
$Result_841008 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\841008.txt'
$Result_842001 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\842001.txt'
$Result_850001 = 'C:\Users\amo31\Google Drive\FX\Trading\Kelpie Project\Portfolio Test\PortfolioResult\850001.txt'

$rateArray = import-csv $Result_840005 -Header OrderNumber, OrderType, OpenTime, CloseTime, OpenPrice, ClosePrice, Lots, Profit, SL, TP, Balance, ID, Pair, Swap
 
#Write-Host $rateArray

 
$rateArray |ForEach-Object {
    
    
    $datetime  = [datetime]::ParseExact($_.Date+'_'+$_.Time,'yyyy.MM.dd_HH:mm',$null)            

    $_.Date = $datetime.ToString("dd/MM/yy HH:mm")
    $_ | Add-Member -MemberType NoteProperty -Name LongSwap -Value 0 -Force 
    $_ | Add-Member -MemberType NoteProperty -Name ShortSwap -Value 0 -Force
    }

 
#Write-Host $rateArray 

$rateArray | 
	select "Date", "Open", "Close", "High", "Low", "Volume","LongSwap","ShortSwap"| 
	ConvertTo-Csv -NoTypeInformation |
    % { $_ -replace '","', ','} | % { $_ -replace "^`"",''} | % { $_ -replace "`"$",''} |
	Select -Skip 1 |
	Out-File  -encoding ASCII "E:\FX\Asirikuy\asirikuy_python_tester_v56_new\asirikuy_python_tester_v56\history\GBPJPY_1M_2017\GBPUSD_1.csv"


 