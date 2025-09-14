local logger = {}

function logger.info(msg, console)
    spdlog.info("[INFO   ] " .. tostring(msg))
    if console then
        print("[IRF][INFO   ] " .. tostring(msg))
    end
end

function logger.warn(msg, console)
    spdlog.info("[WARNING] " .. tostring(msg))
    if console then
        print("[IRF][WARNING] " .. tostring(msg))
    end
end

function logger.error(msg, console)
    spdlog.info("[ERROR  ] " .. tostring(msg))
    if console then
        print("[IRF][ERROR  ] " .. tostring(msg))
    end
end

return logger