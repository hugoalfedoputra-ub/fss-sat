#ifndef TAGS_H
#define TAGS_H

#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <inet/common/geometry/common/Coord.h>

class PowerTag : public inet::TagBase
{
protected:
    double transmitPower_dBm;  // Transmit Power (Pt) in dBm
    double transmitGain_dBi;   // Transmit Antenna Gain (Gt) in dBi
    double eirp_dBm;          // Effective Isotropic Radiated Power (EIRP) in dBm
    double receiveGain_dBi;   // Receive Antenna Gain (Gr) in dBi
    double receivedPower_dBm; // Received Power (Pr) in dBm
    double fspl_dB;           // Free Space Path Loss (FSPL) in dB

public:
    PowerTag() : transmitPower_dBm(0.0), transmitGain_dBi(0.0), eirp_dBm(0.0),
                     receiveGain_dBi(0.0), receivedPower_dBm(0.0), fspl_dB(0.0) {}

    virtual const char* getName() const override { return "PowerTag"; }

    virtual std::string str() const override {
        std::stringstream ss;
        ss << "Pt: " << transmitPower_dBm << " dBm, Gt: " << transmitGain_dBi << " dBi, "
           << "EIRP: " << eirp_dBm << " dBm, Gr: " << receiveGain_dBi << " dBi, "
           << "Pr: " << receivedPower_dBm << " dBm, FSPL: " << fspl_dB << " dB";
        return ss.str();
    }

    virtual PowerTag *dup() const override { return new PowerTag(*this); }

    // Getters and setters for each parameter
    double getTransmitPower_dBm() const { return transmitPower_dBm; }
    void setTransmitPower_dBm(double p) { transmitPower_dBm = p; }

    double getTransmitGain_dBi() const { return transmitGain_dBi; }
    void setTransmitGain_dBi(double g) { transmitGain_dBi = g; }

    double getEIRP_dBm() const { return eirp_dBm; }
    void setEIRP_dBm(double e) { eirp_dBm = e; }

    double getReceiveGain_dBi() const { return receiveGain_dBi; }
    void setReceiveGain_dBi(double g) { receiveGain_dBi = g; }

    double getReceivedPower_dBm() const { return receivedPower_dBm; }
    void setReceivedPower_dBm(double p) { receivedPower_dBm = p; }

    double getFSPL_dB() const { return fspl_dB; }
    void setFSPL_dB(double f) { fspl_dB = f; }

    // Helper function to calculate EIRP (if not set directly)
    void calculateEIRP() { eirp_dBm = transmitPower_dBm + transmitGain_dBi; }
};


class TargetTag : public inet::TagBase
{
private:
    int targetMCC;
    inet::Coord targetPosition; // Add target position

public:
    TargetTag() : targetMCC(-1) {}
    virtual void setTarget(int target) { targetMCC = target; }
    virtual int getTarget() const { return targetMCC; }

    void setPosition(const inet::Coord& pos) { targetPosition = pos; }
    const inet::Coord& getPosition() const { return targetPosition; }

    virtual const char* getName() const override { return "TargetTag"; }
    virtual std::string str() const override {
        std::stringstream ss;
        ss << "Target MCC: " << targetMCC << ", Position: " << targetPosition;
        return ss.str();
    }
    virtual TargetTag* dup() const override { return new TargetTag(*this); }

};

class CarrierTag : public inet::TagBase
{
private:
    double carrierFrequency;
public:
    CarrierTag() : carrierFrequency(0) {}
    virtual void setCarrierFrequency(double freq) { carrierFrequency = freq; }
    virtual CarrierTag *dup() const override { return new CarrierTag(*this); }
    virtual double getCarrierFrequency() const { return carrierFrequency; }
    virtual const char* getName() const override { return "CarrierTag"; }
    virtual std::string str() const override { return ""; }
};

class LatencyTag : public inet::TagBase
{
private:
    inet::simtime_t latency;

public:
    LatencyTag() : latency(0) {}

    virtual const char* getName() const override { return "LatencyTag"; }
    virtual std::string str() const override {
        std::stringstream ss;
        ss << "Latency: " << latency;
        return ss.str();
    }
    virtual LatencyTag* dup() const override { return new LatencyTag(*this); }

    void setLatency(inet::simtime_t l) { latency = l; }
    inet::simtime_t getLatency() const { return latency; }
};

#endif
