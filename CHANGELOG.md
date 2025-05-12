# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)


## [v0.0.0] - 2025-05-12 - 彭程
### Added
- A gitignore file excluding unnecessary large files
- add makefiles in different directories

### Changed
- move data files into `data/`, top `Makefile` into project's root directory
- reorganize directory `code` into `src`, separate header files and cpp files 

### Fixed
- 

### Comments
- **推荐每次提交后在CHANGERLOG.md中进行记录.**
- 为了能快速拉取和上传，不要用git上传构建文件、数据文件和图片到GitHub上，不要将外部依赖库如Eigen放到项目文件夹下进行git上传。(可以git rm --cached 从版本控制中移除文件，但保留文件在本地工作目录中的副本)
- 不要将数据和代码放一块，保持目录的结构清晰。
- 由于我们目前只用改Ada-IVF一个文件，所以可以暂时都在同一个分支上写，写之前确定一下函数分工，如果有冲突就手动解决一下冲突。
.
├── Makefile               # 顶层，列出子模块构建顺序
├── src
│   ├── Makefile           # 构建所有模块
│   ├── clustering/Makefile
│   ├── dim_reduction/Makefile
│   ├── utils/Makefile
│   └── main.cpp

如上，在增添clustering目录下的文件时，只需修改对应目录下的Makefile，如果还需要添加而非覆盖原有编译选项，可以CXXFLAGS += -Wextra...,最后在顶层目录make.
---

