ll -h /mnt/nvme/card0/0
ll -h /mnt/nvme/card0/1

ll -h /mnt/nvme/card1/0
ll -h /mnt/nvme/card1/1

rm /mnt/nvme/card0/0/*
rm /mnt/nvme/card0/1/*

rm /mnt/nvme/card1/0/*
rm /mnt/nvme/card1/1/*

touch /mnt/nvme/card0/0/state1
touch /mnt/nvme/card0/1/state2
touch /mnt/nvme/card1/0/state3
touch /mnt/nvme/card1/1/state4