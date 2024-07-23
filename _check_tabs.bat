@set EMPTY=
@set PROCESS_FILES_MASKS=*.cpp^,*.c^,*.cc^,*.cxx^,*.c++^,*.hpp^,*.h^,*.hxx^,*.h++^,*.md_^ %EMPTY%
@set EXCLUDE_FILES_MASKS=*/.msvc/*^,*/compact_enc_det/*,*/util* %EMPTY%
@umba-tabtool --check --space --scan --include-files=%PROCESS_FILES_MASKS% --exclude-files=%EXCLUDE_FILES_MASKS% .

@rem umba-tabtool --help >umba-tabtool.txt

