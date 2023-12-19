class Notification {
    public:
        enum State {
            none,
            waitingWifi,
            waitingCreds,
            waitingMqtt,
            connected
        };

        Notification(int speakerPin, int redPin, int greenPin, int bluePin);
        void setWaitingWifi();
        void setWaitingCreds();
        void setWaitingMqtt();
        void setConnected();
        void clearState();
        void gateOpened();
        void loop();
        State state;

    private:
        int speakerPin;
        int redPin;
        int greenPin;
        int bluePin;
        bool stateChanged;

        int lastLoop;


        void setLedColour(int red, int green, int blue);
        void playNotification(int notification[], int length);
};