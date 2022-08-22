from winreg import REG_NONE
from regkey import RegKeyItem

def execute_cli(cmd: str, argv: str|list=''):
	from os import popen
	if isinstance(argv, list):
		argv = ' '.join(['"%s"'%(e) for e in argv])
	cmdline = '..\\build\\bin\\cli.exe %s %s'%(cmd, argv)
	return popen(cmdline).read()

def GetUserChoiceHash(ext: str, ProgId: str):
	return execute_cli('UserChoice', [ext, ProgId]).split('\n')[-2].split(':')[-1].strip()

root = RegKeyItem('HKEY_CURRENT_USER\\SOFTWARE\\Classes')
assoc = RegKeyItem('HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts')

extlist = [
	('cpp_source', 'C++ Source', ['cpp', 'cc', 'cxx', 'c++', 'inl'], None),
	('cpp_header', 'C++ Header', ['hpp', 'hh', 'hxx', 'h++'], None),
	('c_source', 'C Source', ['c'], None),
	('c_header', 'C Header', ['h'], None),
	('assembly', 'Assembly Source', ['s', 'asm'], None),
	('java', 'Java Source', ['java'], None),
	('visualbasic', 'VisualBasic Source', ['vb'], None),
	('csharp', 'C# Source', ['cs'], None),
	('cmake', 'CMake Script', ['cmake'], None),
	('plain', 'Plain Text', ['txt'], None),
	('ini', 'INI Settings', ['ini'], None),
	('xml', 'XML Document', ['xml'], None),
	('json', 'JSON Document', ['json'], None),
	('rust', 'Rust Source', ['rs'], None),
	('yaml', 'YAML Document', ['yaml', 'yml'], None),
	('typescript', 'TypeScript Source', ['ts'], None),
	('pylauncher', 'Python Source', ['py'], 'E:\\EnvrSupport\\Python\\python310\\python.exe'),
	('jslauncher', 'JavaScript Source', ['js'], 'E:\\EnvrSupport\\Node.js\\node-v18.0.0-win-x64\\node.exe'),
]

for id, brief, exts, exec in extlist:
	# register progId
	progId = 'IWinDEModifier.%s'%(id)
	key = root[progId].set(brief)
	key['DefaultIcon'] = 'E:\\DesktopMap\\dev\\IWinDEModifier\\Data\\Local\\FileExt\\%s.ico,0'%(brief)
	# register default shell verb
	shell = key['Shell'].set('Edit')
	tmp = shell['Edit'].set('MUIVerb', '编辑(&E)')['Command'].set('"E:\PortableApp\VSCode\Code.exe" "%L"')
	if exec is not None:
		tmp = shell.set('Execute')['Execute'].set('MUIVerb', '执行')['Command'].set('"%s" "%%L"'%(exec))
	# register
	for ext in exts:
		ext = '.%s'%(ext)
		hash = GetUserChoiceHash(ext, progId)
		tmp = root[ext].set(progId)['OpenWithProgids']
		tmp = assoc[ext]['OpenWithProgids'].set(progId, type=REG_NONE)
		tmp = assoc[ext]['UserChoice'].set('ProgId', progId).set('Hash', hash)

execute_cli('flush')

# DeclareProg('IWinDEModifier.pylauncher',
# 	brief ='IWinDEModifier Python Script',
# 	icon  ='C:\\Users\\Nian\\Desktop\\icon\\ext-py.ico,0') \
# 	.DeclareShell('Execute', alias='执行', cmd='"%PYTHON_HOME%\python.exe" "%L"', expand=True) \
# 	.DeclareShell('Edit', alias='编辑', cmd='"E:\PortableApp\VSCode\Code.exe" "%L"') \
# 	.SetDefaultShell('Execute') \
# 	.Register()


# 此电脑(×) → 颜表情轮盘(√)
ThisPC = RegKeyItem('HKEY_CURRENT_USER\\SOFTWARE\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}')

ThisPC.set('LocalizedString', 'This PC')
execute_cli('flush')