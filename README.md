Version 1.1 (01.02.2017)

GPULoadAlert is a simple tool to monitor NVIDIA GPU usage and alerting the user when the GPU load falls below a 
certain threshold. The purpose of this tool is to be notified when the GPU is idle after being finished with a
rendering job.


Using GPULoadAlert:
At the top of the menu are buttons to save or restore the current values from data/settings.xml
Monitor GPU toggle: Starts or stops the monitoring of the selected GPU
Periodical Alert toggle: If checked, the monitoring will continue until it is turned off manually, if it is
unchecked the alert will only trigger once
Which GPU #: Select the actual GPU to monitor (if multiple are installed in the system) by dragging in this
field
Use Mail toggle: When the alert is triggered a mail is sent with the account specified in data/smtp.xml
Use Alert Sound toggle: When the alert is triggered a sound will be played
Wait Time (Minutes): The time for the application to wait before triggering an alert. Useful if the GPU load of
the rendering job drops sometimes. Only if the GPU is below the threshold for the set amount of minutes the
alert will be triggered. Set the time by dragging in this field
Minimum GPU Load: Set the % threshold for the alert to trigger by dragging in this field

The default values can be edited manually in data/settings.xml
While monitoring the GPU the interface will be tinted green, otherwise red.


![App screenshot when inactive](https://raw.githubusercontent.com/Echolaitoc/GPULoadAlert/master/inactive.png)
When the app is inactive

![App screenshot when active](https://raw.githubusercontent.com/Echolaitoc/GPULoadAlert/master/active.png)
When the app is monitoring the GPU

About GPULoadAlert source:
This is an openframeworks 0.9.8 project. It depends on the addons ofxGui, ofxSMTP and ofxSSLManager
Download: https://github.com/Echolaitoc/GPULoadAlert

Download compiled application: http://tristanweber.com/GPULoadAlert.zip

Questions or bug reports to: info@tristanweber.com
