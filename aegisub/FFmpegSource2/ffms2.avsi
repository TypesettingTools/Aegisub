# FFmpegSource 1.21 syntax compatibility
# Created by TheFluff

function FFmpegSource2(string source, int "vtrack", int "atrack", bool "cache", \
	string "cachefile", int "fpsnum", int "fpsden", string "pp", int "threads", \
	string "timecodes", int "seekmode", bool "overwrite") {
	
	vtrack 		= default(vtrack,-1)
	atrack		= default(atrack,-2)
	cache		= default(cache,true)
	cachefile	= default(cachefile,source+".ffindex")
	fpsnum		= default(fpsnum,-1)
	fpsden		= default(fpsden,1)
	pp		= default(pp,"")
	threads		= default(threads,-1)
	timecodes	= default(timecodes,"")
	seekmode	= default(seekmode,1)
	overwrite	= default(overwrite,false)
	
	((cache == true) && (atrack <= -2)) ? ffindex(source=source, cachefile=cachefile, \
		indexmask=0, overwrite=overwrite) : (cache == true) ? ffindex(source=source, \
		cachefile=cachefile, indexmask=-1, overwrite=overwrite) : nop
	
	v = ffvideosource(source=source, track=vtrack, cache=cache, cachefile=cachefile, \
		fpsnum=fpsnum, fpsden=fpsden, pp=pp, threads=threads, timecodes=timecodes, \
		seekmode=seekmode)
	
	a = (atrack <= -2) ? blankclip(audio_rate=0) : ffaudiosource(source=source, \
		track=atrack, cache=cache, cachefile=cachefile)
	
	return audiodubex(v,a)
}
