ll -h /mnt/nvme/card0/0
ll -h /mnt/nvme/card0/1
ll -h /mnt/nvme/card0/2
ll -h /mnt/nvme/card0/3

ll -h /mnt/nvme/card1/0
ll -h /mnt/nvme/card1/1
ll -h /mnt/nvme/card1/2
ll -h /mnt/nvme/card1/3

rm /mnt/nvme/card0/0/*
rm /mnt/nvme/card0/1/*
rm /mnt/nvme/card0/2/*
rm /mnt/nvme/card0/3/*

rm /mnt/nvme/card1/0/*
rm /mnt/nvme/card1/1/*
rm /mnt/nvme/card1/2/*
rm /mnt/nvme/card1/3/*

touch /mnt/nvme/card0/0/state1
touch /mnt/nvme/card0/1/state2
touch /mnt/nvme/card0/2/state3
touch /mnt/nvme/card0/3/state4
touch /mnt/nvme/card1/0/state5
touch /mnt/nvme/card1/1/state6
touch /mnt/nvme/card1/2/state7
touch /mnt/nvme/card1/3/state8