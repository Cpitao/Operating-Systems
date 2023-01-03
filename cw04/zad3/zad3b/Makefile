normal: catcher_normal sender_normal

realtime: catcherrt senderrt

catcher_normal:
	gcc catcher.c -o catcher

sender_normal:
	gcc sender.c -o sender

catcherrt:
	gcc catcher.c -o catcher -D RTSIG

senderrt:
	gcc sender.c -o sender -D RTSIG
clean:
	rm sender catcher
