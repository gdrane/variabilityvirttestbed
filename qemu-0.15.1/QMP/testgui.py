import easygui as eg
import os
import sys
import json
from qmp import QEMUMonitorProtocol

eg.msgbox("Variability Module", '', 'OK', 'cutils.png', None)

msg = "Enter No of Instruction classes"
total_insn_classes = eg.integerbox(msg, 'Variability Info', 1, 0, 6, None, None) 
msg = 'Enter No of Power Calculation parameters'
total_model_params = eg.integerbox(msg, '', 2, 0, 20, None, None)
i = 1
variability_input = {}
insn_classes = []
while i <= total_insn_classes:
	msg = 'Enter Instruction Class Information'
	fieldNames = ["Name of Instruction Class", "a", "b"]
	fieldValues = eg.multenterbox(msg, '', fieldNames)
	print fieldValues
	while 1:
		if fieldValues == None: break
		errmsg = ""
		for j in range(len(fieldNames)):
			if fieldValues[i].strip() == "":
				errmsg += ('"%s" is a required field.\n\n' % fieldNames[i])
		if errmsg == "":
			break
		fieldValues = eg.multenterbox(errmsg, '', fieldNames, fieldValues)
	insn_class_input = {}
	insn_class_input['idx'] = i
	insn_class_input['class_name'] = fieldValues[0].strip()
	insn_classes.append(fieldValues[0].strip())
	insn_class_input['a'] = fieldValues[1].strip()
	insn_class_input['b'] = fieldValues[2].strip()
	variability_input[i] = insn_class_input
	i = i + 1
insn_class_input = {}
insn_class_input['idx'] = i
insn_class_input['class_name'] = 'rest'
insn_class_input['a'] = 0
insn_class_input['b'] = 0
variability_input[i] = insn_class_input
power_params = json.dumps(variability_input)
arch_name = eg.enterbox('Enter Name of Architecture')
if arch_name == None:
	sys.exit(0)
fh = open(arch_name + '_instruction_list.txt', 'r')
i = 1
insn_class_info = {}
insns = []
for insn in fh:
	insns.append(insn)
fh.close()
for_class = insns[:]
for insn_class in insn_classes:
	msg = 'Select instructions in class ' + insn_class
	choices = eg.multchoicebox(msg, '', for_class)
	insn_class_info[insn_class] = choices
	for choice in choices:
		for_class.remove(choice)
insn_class_info['rest'] = for_class
insn_error_info = {}
msg = 'Select Errorneous instructions '
choices = eg.multchoicebox(msg, '', insns)
insn_error_info['y'] = choices
for_class = insns[:]
for choice in choices:
	for_class.remove(choice)
insn_error_info['n'] = for_class
"""
for insn in fh:
	instruction = {}
	instruction['name'] = insn
	msg = 'For instruction ' + insn + ' Enter Details:'
	title = 'Instruction Class'
	choices = insn_classes
	choice = eg.choicebox(msg, title, choices)
	if choice == None:
		sys.exit(0)
	instruction['insn_class'] = choice
	title = 'Instruction Errorneous or not'
	choices = ["Yes" , "No"]
	choice = eg.choicebox(msg, title, choices)
	if choice == None:
		sys.exit(0)
	instruction['error_insn'] = choice
	insn_info[i] = instruction 
"""
insn_class_params = json.dumps(insn_class_info)
insn_error_params = json.dumps(insn_error_info)
file_handle = open('./variability_input_data.txt', 'w')
file_handle.write(power_params)
file_handle.write('\n')
file_handle.write(insn_class_params)
file_handle.write('\n')
file_handle.write(insn_error_params)
file_handle.close()

#eg.msgbox(data_string)
os.system('../arm-softmmu/qemu-system-arm -M lm3s811evb -kernel ../../../sleepexp/main.elf -qmp unix:../qmp-sock,server')
msg = "Do you want to continue?"
title = "Please Confirm"
if eg.ccbox(msg, title):     # show a Continue/Cancel dialog
	pass  # user chose Continue
else:
	sys.exit(0)           # user chose Cancel
