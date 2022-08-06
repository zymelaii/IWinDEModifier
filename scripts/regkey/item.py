from winreg import *

RegTopClassDict = {
	'HKEY_CLASSES_ROOT': HKEY_CLASSES_ROOT,
	'HKEY_CURRENT_USER': HKEY_CURRENT_USER,
	'HKEY_LOCAL_MACHINE': HKEY_LOCAL_MACHINE,
	'HKEY_USERS': HKEY_USERS,
	'HKEY_PERFORMANCE_DATA': HKEY_PERFORMANCE_DATA,
	'HKEY_CURRENT_CONFIG': HKEY_CURRENT_CONFIG,
	'HKEY_DYN_DATA': HKEY_DYN_DATA,
}

RegValueTypeDict = {
	REG_BINARY: "REG_BINARY",
	REG_DWORD: "REG_DWORD",
	REG_DWORD_LITTLE_ENDIAN: "REG_DWORD_LITTLE_ENDIAN",
	REG_DWORD_BIG_ENDIAN: "REG_DWORD_BIG_ENDIAN",
	REG_EXPAND_SZ: "REG_EXPAND_SZ",
	REG_LINK: "REG_LINK",
	REG_MULTI_SZ: "REG_MULTI_SZ",
	REG_NONE: "REG_NONE",
	REG_QWORD: "REG_QWORD",
	REG_QWORD_LITTLE_ENDIAN: "REG_QWORD_LITTLE_ENDIAN",
	REG_RESOURCE_LIST: "REG_RESOURCE_LIST",
	REG_FULL_RESOURCE_DESCRIPTOR: "REG_FULL_RESOURCE_DESCRIPTOR",
	REG_RESOURCE_REQUIREMENTS_LIST: "REG_RESOURCE_REQUIREMENTS_LIST",
	REG_SZ: "REG_SZ",
}

class RegKeyItem:
	def __init__(self, path: str, key=None):
		global RegTopClassDict
		deps = [e.strip() for e in path.split('\\') if len(e) > 0]
		if key is not None:
			if len(path) == 0:
				self.key = key
			else:
				self.key = OpenKeyEx(key, '\\'.join(deps), access=KEY_ALL_ACCESS | KEY_WOW64_64KEY)
		else:
			self.key = OpenKeyEx(RegTopClassDict[deps[0]], '\\'.join(deps[1:]), access=KEY_ALL_ACCESS | KEY_WOW64_64KEY)
	def __getitem__(self, subkey: str):
		try:
			key = OpenKeyEx(self.key, subkey, access=KEY_READ | KEY_WRITE | KEY_WOW64_64KEY)
		except:
			CreateKeyEx(self.key, subkey, access=KEY_ALL_ACCESS | KEY_WOW64_64KEY)
			key = OpenKeyEx(self.key, subkey, access=KEY_ALL_ACCESS | KEY_WOW64_64KEY)
		return RegKeyItem('', key)
	def __setitem__(self, subkey: str, value: str):
		key = self.__getitem__(subkey).key
		SetValueEx(key, '', 0, REG_SZ, value)
	def set(self, key_or_value: str, value=None, type=REG_SZ):
		if value is None and type is REG_SZ:
			SetValueEx(self.key, '', 0, type, key_or_value)
		else:
			SetValueEx(self.key, key_or_value, 0, type, value)
		return self
	def keys(self):
		keylist, id = ([], 0)
		while True:
			try:
				keyname = EnumKey(self.key, id)
				keylist.append(keyname)
				id += 1
			except OSError:
				break
		return keylist
	def values(self):
		global RegValueTypeDict
		values, id = ([], 0)
		while True:
			try:
				value = EnumValue(self.key, id)
				values.append((value[0], value[1], RegValueTypeDict[value[2]]))
				id += 1
			except OSError:
				break
		return values
	def remove(self, subkey: str):
		deps = [e.strip() for e in subkey.split('\\') if len(e) > 0]
		while True:
			item = self['\\'.join(deps)]
			keys = item.keys()
			for key in keys:
				item.remove(key)
			if len(deps) <= 1:
				break
			deps.pop()
		DeleteKeyEx(self.key, subkey, access=KEY_ALL_ACCESS | KEY_WOW64_64KEY)
	def erase(self, value: str):
		try:
			DeleteValue(self.key, value)
		except FileNotFoundError:
			pass
		return self