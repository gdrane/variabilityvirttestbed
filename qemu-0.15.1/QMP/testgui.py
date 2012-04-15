import easygui as eg
import os
import sys
import json
from qmp import QEMUMonitorProtocol

eg.msgbox("Variability Module", '', 'OK', 'cutils.png', None)

msg = "Enter No of Instruction classes"
total_insn_classes = eg.integerbox(msg, 'Variability Info', 1, 0, 6, None, None) 
i = 1
variability_input = {}
while i <= total_insn_classes:
	msg = 'Enter Instruction Class Information'
	fieldNames = ["Name of Instruction Class", "a", "b"]
	fieldValues = eg.multenterbox(msg, '', fieldNames)
	while 1:
		if fieldValues == None: break
		errmsg = ""
		for i in range(len(fieldNames)):
			if fieldValues[i].strip() == "":
				errmsg += ('"%s" is a required field.\n\n' % fieldNames[i])
		if errmsg == "":
			break
		fieldValues = multenterbox(errmsg, '', fieldNames, fieldValues)
	insn_class_input = {}
	insn_class_input['idx'] = i
	insn_class_input['class_name'] = fieldValues[0].strip()
	insn_class_input['a'] = fieldValues[1].strip()
	insn_class_input['b'] = fieldValues[2].strip()
	variability_input[i] = insn_class_input
	i = i + 1
data_string = json.dumps(variability_input)
file_handle = open('./variability_input_data.txt', 'w')
file_handle.write(data_string)
file_handle.close()

eg.msgbox(data_string)
os.system('../arm-softmmu/qemu-system-arm -M lm3s811evb -kernel ../../../sleepexp/main.elf -qmp unix:../qmp-sock,server')
msg = "Do you want to continue?"
title = "Please Confirm"
if eg.ccbox(msg, title):     # show a Continue/Cancel dialog
	pass  # user chose Continue:wq
else:
	sys.exit(0)           # user chose Cancel
