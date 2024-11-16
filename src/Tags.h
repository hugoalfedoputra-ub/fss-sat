#ifndef TAGS_H
#define TAGS_H

#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <inet/common/geometry/common/Coord.h>

class PowerTag : public inet::TagBase
{
protected:
    double power_dBm;

public:
    PowerTag() : power_dBm(0.0) {}
    PowerTag(double p) : power_dBm(p) {}
    virtual const char* getName() const override { return "PowerTag"; }
    virtual std::string str() const override { return std::to_string(power_dBm) + " dBm"; }
    virtual PowerTag *dup() const override { return new PowerTag(*this); }

    double getPower_dBm() const { return power_dBm; }
    void setPower_dBm(double p) { power_dBm = p; }

    double getPower_mW() const { return pow(10.0, power_dBm / 10.0); }  // Conversion to mW
    void setPower_mW(double p) { power_dBm = 10.0 * log10(p); }        // Conversion from mW
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

#endif
