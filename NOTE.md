# Registry Configure Note

- HKEY_CLASSES_ROOT $\sim$ HKEY_CURRENT_USER\Software\Classes $\cup$ HKEY_LOCAL_MACHINE\Software\Classes
- Order of Shell verbs
	- default verb
	- first verb if order is specified
	- Open
	- OpenWith
-  shell item of cascade menu stores in *\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell

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

```plain
<ProgID>\
	(Default) = <FileExt Info (show in property panel)>
	[AlwaysShowExt = <Show FileExt (followed after FileExt info)>]
	[Content Type = <MIME Content Type>]
	[PerceivedType = <Perceived Type>]
	[FriendlyTypeName = <Friendly Type Name>]
	DefaultIcon\
		(Default) = <Default Icon>
	Shell\
		[(Default) = <Default Verb>]
		<verb>\
			[(Default) = <Default Verb Name>]
			[MUIVerb = <Verb Alias>]
			[Icon = <Menu Icon>]
			Command
				(Default) = <Execute Command>
			DropTarget
				CLSID = <Class ID>
		[<verb 2>]
		[...]

.<extension>\
	(Default) = <Default ProgID>
	[Content Type = <MIME Content Type>]
	[PerceivedType = <Perceived Type>]
	OpenWithProgids\
		[<Spare ProgID>]
		[...]
```

```ini
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