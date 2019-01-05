
curlsmtpps.dll: dlldata.obj curlsmtp_p.obj curlsmtp_i.obj
	link /dll /out:curlsmtpps.dll /def:curlsmtpps.def /entry:DllMain dlldata.obj curlsmtp_p.obj curlsmtp_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del curlsmtpps.dll
	@del curlsmtpps.lib
	@del curlsmtpps.exp
	@del dlldata.obj
	@del curlsmtp_p.obj
	@del curlsmtp_i.obj
