C:\WinDDK\7600.16385.1\tools\devcon\i386\devcon.exe remove HID\vmulti 
cd .. 
cmd /c buildme.bat 
cd bin 
cmd /c install_driver.bat 
echo After keypress, launching testvmulti 
pause 
testvmulti /joystick 
