#Remember to use ssh-keyscan <SERVER> > ~/.ssh/known_hosts to persist the SFTP server fingerprint to avoid interactive prompting

SFTP_SERVER=$RPI_IP
SFTP_USER=$RPI_USER
SFTP_PWD=$RPI_PASSWORD
currentdir=$(pwd)

if [[ -z "$SFTP_SERVER" ]]; then
   SFTP_SERVER="192.168.2.90"
fi

if [[ -z "$SFTP_USER" ]]; then
   SFTP_USER="pi"
fi

if [[ -z "$SFTP_PWD" ]]; then
   SFTP_PWD="raspberry"
fi

pscp -batch -pw "$SFTP_PWD" $currentdir/bin/PI/SDL2GuiHelperSample.bin pi@$SFTP_SERVER:/home/pi/devtest/SDL2GuiHelper/bin/PI/
echo "Starting GdbServer on Raspberry PI"
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'cd /home/pi/devtest/SDL2GuiHelper/bin/PI/'
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'chmod 771 /home/pi/devtest/SDL2GuiHelper/bin/PI/SDL2GuiHelperSample.bin'
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'gdbserver localhost:9999 /home/pi/devtest/SDL2GuiHelper/bin/PI/SDL2GuiHelperSample.bin'
echo "GdbServer on Raspberry PI stopped, ready for next session."
