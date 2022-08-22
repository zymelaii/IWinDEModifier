# Registry Configure Note

- `HKEY_CLASSES_ROOT` $\sim$ `HKEY_CURRENT_USER\Software\Classes` $\cup$ `HKEY_LOCAL_MACHINE\Software\Classes`
- Order of Shell verbs
	- default verb
	- first verb if order is specified
	- Open
	- OpenWith
-  shell item of cascade menu stores in `*\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell`
- `*\SOFTWARE\Microsoft\Windows Portable Devices\FormatMap` and `*\SOFTWARE\WOW6432Node\Microsoft\Windows Portable Devices\FormatMap` contains FileExts
- `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace`包含资源管理器初始界面的文件夹显示控制，本电脑通用
- `*\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Desktop\NameSpace`用于控制显示在资源管理器侧边栏的自定义文件夹/其他链接，被列入该项的子项的命名应当能够在`*\SOFTWARE\Classes\CLSID`中找到

## Brief

```yaml
Target:
	- HKEY_CLASSES_ROOT:
		Access: ReadOnly
	- HKEY_LOCAL_MACHINE:
		Access: ReadOnly
	- HKEY_CURRENT_USER:
		Access: ReadAndWrite
ModifiyLocation:
	Root:
		- HKEY_LOCAL_MACHINE
		- HKEY_CURRENT_USER
	Path:
		- SOFTWARE\Classes
		- SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts
Item:
	- Program:
		Format: <prog-id>
		Location:
			- SOFTWARE\Classes
	- FileExt:
		Format: .<file-ext>
		Location:
			- SOFTWARE\Classes
			- SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts
	- ShellVerb:
		Format: <verb>
		Location:
			- ${Program}\Shell
```

## Value Settings

```yaml
NoOpenWith:
	Brief: 从打开方式中排除该项
	Type: REG_SZ
	Flag:
		- NoValue
		- Optional
	Owner:
		- Program
Extended:
	Brief: 仅当用户右键单击某个对象的同时按 SHIFT 键时，才显示关联的命令
	Type: REG_SZ
	Flag:
		- NoValue
		- Optional
	Owner:
		- ShellVerb
Extended:
	Brief: 永远不会显示在上下文菜单中，仅可通过编程手段访问
	Type: REG_SZ
	Flag:
		- NoValue
		- Optional
	Owner:
		- ShellVerb
```

```yaml
# Shell文件夹类项格式
+ <GUID>
	- DescriptionID: <Description ID -> REG_DWORD>
	- InfoTip: <Info Tip> # 悬浮时显示的提示信息
	- System.IsPinnedToNameSpaceTree: <Pinned To NameSpace Tree -> REG_DWORD> # 标记是否附加到导航窗格
	+ DefaultIcon # 自定义默认图标
		- (Default): <Icon URL>
	+ InProcServer32 # 标记容纳COM类的载体是为DLL
		- (Default): <DLL Path>
		- ThreadingModel: <Threading Model> # threading model of dll, e.g. Apartment, Both, ...
	+ Instance
		- CLSID: <GUID of Instance>
		+ InitPropertyBag
			- Attributes: <Attributes -> REG_DWORD>
			- TargetKnownFolder: <GUID of Target Folder>
	+ ShellFolder
		- Attributes: <Attributes -> REG_DWORD>
		- FolderValueFlags: <Folder Value Flags -> REG_DWORD>
		- SortOrderIndex: <Index -> REG_DWORD>

# 程序类项
+ <ProgID>
	- (Default): <FileExt Info (show in property panel)>
	- AlwaysShowExt: <Show FileExt (followed after FileExt info)> # 可选
	- Content Type: <MIME Content Type> # 可选
	- PerceivedType: <Perceived Type> # 可选
	- FriendlyTypeName: <Friendly Type Name> # 可选
	+ DefaultIcon
		- (Default): <Icon URL>
	+ Shell
		- (Default): <Default Verb>
		+ <Verb...>
			- (Default): <Default Verb Name> # 可选
			- MUIVerb: <Verb Alias> # 可选
			- Icon: <Menu Icon> # 可选
			+ Command
				- (Default): <Execute Command>
			+ DropTarget
				- CLSID: <GUID of Target>

# 拓展名项
+ .<extension>
	- (Default): <Default ProgID> # 可选
	- Content Type: <MIME Content Type> # 可选
	- PerceivedType: <Perceived Type> # 可选
	+ OpenWithProgids
		- <Spare ProgID...>

# ShellFolder NameSpace 导航窗格白名单
+ SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\AllowedEnumeration
	- <GUID Shell Folder...>: <Enabled -> REG_DWORD>

# 快速访问属性配置
+ SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer
	- HubMode: <Remove Quick Access -> REG_DWORD>
	- ShowRecent: <Enabled -> REG_DWORD>
	- ShowFrequent: <Enabled -> REG_DWORD>
	+ Advanced
		- LaunchTo: <Shell Folder Launched to on Open -> REG_DWORD>
			# 0: None
			# 1: This PC
			# 2: Quick Access
			# 3: This PC/Downloads
```

```yaml
# Explorer: Navigation Example
+ HKEY_LOCAL_MACHINE\SOFTWARE
	+ Classes\CLSID
		+ {20d04fe0-3aea-1069-a2d8-08002b30309d}
			- (Default): This PC
			- InfoTip: 
			- LocalizedString: @%SystemRoot%\system32\windows.storage.dll,-9216 # 此电脑
			- System.Keywords: @%windir%\system32\windows.storage.dll,-9012 # 计算机
			- System.PropList.DetailsPaneNullSelect: prop:*System.Computer.DomainName;*System.Computer.Workgroup;*System.Computer.Processor;System.Computer.Memory
			- System.PropList.DetailsPaneNullSelectTitle: prop:System.Computer.SimpleName;*System.Computer.Description
		+ {24ad3ad4-a569-4530-98e1-ab02f9417aa8}
			- DescriptionID: 0x00000003
			- Infotip: @C:\Windows\system32\shell32.dll,-12688
			- System.IsPinnedToNameSpaceTree: 0x00000001
			+ DefaultIcon
  				- (Default): C:\Windows\system32\imageres.dll,-113
			+ InProcServer32
  				- (Default): C:\Windows\system32\shell32.dll
  				- ThreadingModel: Both
			+ Instance
  				- CLSID: {0E5AAE11-A475-4c5b-AB00-C66DE400274E}
				+ InitPropertyBag
					- Attributes: 0x00000011
					- TargetKnownFolder: {0ddd015d-b06c-45d5-8c4c-f59713854639}
			+ ShellFolder
  				- Attributes: 0xf080004d
  				- FolderValueFlags: 0x00000029
  				- SortOrderIndex: 0x00000000
		+ {0E5AAE11-A475-4c5b-AB00-C66DE400274E}
			- (Default): Shell File System Folder
			+ InProcServer32
				- (Default): %SystemRoot%\system32\Windows.Storage.dll
				- ThreadingModel: Both
			+ ShellFolder
				- FolderValueFlags: 0x00080000
	+ Microsoft\Windows\CurrentVersion\Explorer
		+ MyComputer\NameSpace
			+ {24ad3ad4-a569-4530-98e1-ab02f9417aa8}
		+ FolderDescriptions
			+ {0ddd015d-b06c-45d5-8c4c-f59713854639}
				- Attributes: 0x00000001
				- Category: 0x00000004
				- Icon: %SystemRoot%\system32\imageres.dll,-113
				- InfoTip: @%SystemRoot%\system32\shell32.dll,-12688
				- LocalizedName: @%SystemRoot%\system32\shell32.dll,-21779
				- Name: Local Picture
				- ParsingName: shell:::{20d04fe0-3aea-1069-a2d8-08002b30309d}\::{24ad3ad4-a569-4530-98e1-ab02f9417aa8}
				- PreCreate: 0x00000001
				- RelativePath: Pictures
				+ PropertyBag
					- BaseFolderId: {33E28130-4E1E-4676-835A-98395C3BC3BB}
					- ThisPCPolicy: Show
			+ {33E28130-4E1E-4676-835A-98395C3BC3BB}
				- Attributes: 0x00000001
				- Category: 0x00000004
				- Icon: %SystemRoot%\system32\imageres.dll,-113
				- InfoTip: @%SystemRoot%\system32\shell32.dll,-12688
				- LocalizedName: @%SystemRoot%\system32\shell32.dll,-21779
				- Name: My Picture
				- ParsingName: shell:::{20d04fe0-3aea-1069-a2d8-08002b30309d}\::{3add1653-eb32-4cb0-bbd7-dfa0abb5acca}
				- PreCreate: 0x00000001
				- RelativePath: Pictures
				- Roamable: 0x00000001
				+ PropertyBag
					- PerferredFolder: {a990ae9f-a03b-4e80-94bc-9912d7504104}
					- ThisPCPolicy: Hide
			+ {A990AE9F-A03B-4E80-94BC-9912D7504104}
				- Category: 0x00000004
				- Icon: C:\Windows\system32\imageres.dll,-1003
				- InfoTip: @C:\Windows\system32\shell32.dll,-12688
				- LocalizedName: @C:\Windows\system32\windows.storage.dll,-34595
				- Name: PicturesLibrary
				- ParentFolder: {1B3EA5DC-B587-4786-B4EF-BD1DC332AEAE}
				- ParsingName: ::{031E4825-7B94-4dc3-B131-E946B44C8DD5}\{A990AE9F-A03B-4e80-94BC-9912D7504104}
				- PreCreate: 0x00000001
				- RelativePath: Pictures.library-ms
				- Stream: 0x00000001
				- StreamResource: C:\Windows\system32\shell32.dll,-3
				- StreamResourceType: LIBRARY
				+ PropertyBag
  					- FoldersDependentOn: {33E28130-4E1E-4676-835A-98395C3BC3BB};{B6EBFB86-6907-413C-9AF7-4FC2ABF07CC5}
```

```ini
# 约束补充
Content Type: <type>/<subtype>
	- <type> -> Optional{text, image, audio, video, application}

PerceivedType: <type>
	- <type> -> Optional{Folder, Text, Image, Audio, Video, Compressed, Document, System, Application, Gamemedia, Contacts}

MUIVerb: @<path>,-<resID>
MUIVerb: <plain>

FriendlyTypeName: @<path>,-<resID>
FriendlyTypeName: <plain>

AlwaysShowExt:
NeverShowExt:

DefaultIcon: <path>,-<resID>
```