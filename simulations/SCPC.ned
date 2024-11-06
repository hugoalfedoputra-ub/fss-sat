package sat2.simulations;

simple SCPCChannel
{
    parameters:
        double bandwidthHz @unit(Hz) = default(36MHz);        // Channel bandwidth
        double symbolRate @unit(Hz) = default(30MHz);        // Symbol rate
        string modulation = default("QPSK");                  // Modulation scheme
        double rolloffFactor = default(0.2);                  // Roll-off factor
        double carrierFrequency @unit(Hz);                    // Carrier frequency
        
    gates:
        input in;
        output out;
}
