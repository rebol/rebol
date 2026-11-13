rebol []
print ["launcher started" now/time/precise]
launch/wait %launched.r3
print ["launcher completed" now/time/precise]
