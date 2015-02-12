UTF-8N


# overview

- MsgBox表示抑制 (重力Errorはx)
- font変更
- chatlog (強制)
- registryのsc情報をAID単位で保存できるように拡張
- FileI/Oをunicode対応へ拡張
- 状態遷移msg拡張 (強制)

# install

ijl15.dllのbackupを取ったらあとは自分を信じて.
ijl15.iniがcurrentになければoriginalのijl15.dllの機能のみを提供

# bug or ...

- XP/w2k3以外ではPacketLengthTableの自動解析が正常に動かない
  "plt_address"に俗に言うPacketLengthを叩き込むと道は開ける、かも

# advanced messages

- SA_FLAMELAUNCHER
- SA_FROSTWEAPON
- SA_LIGHTNINGLOADER
- SA_SEISMICWEAPON
- LK_BERSERK
- HP_ASSUMPTIO
- TK_SEVENWIND
- SL_KAITE
- SL_KAUPE
- SOULLINKs
- TRUESIGHT

- Elemental Converter
- Cursed Water

# work in progress

# diff

- inifileのfontiszeの扱いをpointからpixelに変更
- APIHook時に一時的にmainthreadを止めるように変更
- converter/water使用時の状態アイコンがサーバー間移動で消えないように修正
