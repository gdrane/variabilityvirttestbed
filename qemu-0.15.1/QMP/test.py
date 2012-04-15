from qmp import QEMUMonitorProtocol

def main(argv=None):
	x = QEMUMonitorProtocol("../qmp-sock", False)
	print x.connect()
	print x.cmd("query-cyclecount")

if __name__ == "__main__":
	main()
