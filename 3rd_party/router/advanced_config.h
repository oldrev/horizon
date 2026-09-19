#pragma once

class ADVANCED_CFG
{
public:
    static const ADVANCED_CFG &GetCfg()
    {
        static const ADVANCED_CFG cfg;
        return cfg;
    }

    double m_MaxTangentAngleDeviation = 1.0;
    double m_MaxTrackLengthToKeep = 0.0005;
    bool m_EnableRouterDump = false;
    int m_FollowBranchTimeout = 50;
    int m_PNSProcessClusterTimeout = 50;
};

