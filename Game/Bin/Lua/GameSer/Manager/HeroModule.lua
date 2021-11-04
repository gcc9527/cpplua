

HeroMgr = {}

function HeroMgr.addHero(heroId)
    local conf = gcfg.hero[heroId]
    if not conf then
        return {}
    end

    local attr = {}
    for _, v in pairs(conf.attr) do
        attr[v.id] = attr[v.id] or 0 + attr[v.num]
    end
    return attr
end

function HeroMgr.getHeroAttr(heroId)
    local conf = gcfg.hero[heroId]
    if not conf then
        return {}
    end

    local attr = {}
    for _, v in pairs(conf.attr) do
        attr[v.id] = attr[v.id] or 0 + attr[v.num]
    end
    return attr
end