package link

type Heartbeat struct {
	Code     uint8
	Version  uint8
	DeviceId uint8
}

type Input struct {
	Code      uint8
	Reference uint8
	Channels  [6]int8
}

type Pong struct {
	Code uint8
}

type Request struct {
	Code      uint8
	Requested uint8
}

type Version struct {
	Code    uint8
	Version [18]byte
}

type State struct {
	Code  uint8
	Flags uint8
}

type Demand struct {
	Code     uint8
	Flags    uint8
	Channels [6]int8
}

type Counters struct {
	Code     uint8
	Demands  uint8
	Sent     uint8
	Received uint8
	RxErrors uint8
}

type IMU struct {
	Code uint8
	Id uint8
	Accels [3]int16
	Gyros [3]int16
}
