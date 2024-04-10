(
~birdhousePort = 8000;
n = NetAddr.new("127.0.0.1", ~birdhousePort);
)

n.sendMsg("/1/value", 0.9)
