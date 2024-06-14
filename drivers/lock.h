#ifndef _LOCK_H_
#define _LOCK_H_

#define PMU_I2C_USER_ID       0x10
#define FLASH_LED_I2C_USER_ID 0x20

// function pointer
bool (*LockCtl)(uint8_t user_id, bool lock, bool wait);

// implement
static uint8_t i2c_lock_user = 0; // zero is not valid!
static bool i2c_locked = false;
static bool nrf_i2c_lock_ctl(uint8_t user_id, bool lock, bool wait)
{
    // Note: there could be a shorter / "smarter" way to write this
    //       but this way make the logic easier to understand

    // PRINT_CURRENT_LOCATION();
    NRF_LOG_INFO("UserID: 0x%02x, LockVar: %u, Wait: %u", user_id, lock, wait);

    if ( user_id == 0 )
        return false;

    if ( lock )
    {
        // try lock
        while ( true )
        {
            if ( !i2c_locked )
            {
                // not locked
                i2c_lock_user = user_id;
                i2c_locked = true;
                NRF_LOG_INFO("Was unlocked, locked");
                return true;
            }
            else
            {
                // locked
                if ( i2c_lock_user == user_id )
                {
                    NRF_LOG_INFO("Locked, same user, skip");
                    return true;
                }
                else
                {
                    if ( wait )
                    {
                        NRF_LOG_INFO("Locked, different user, trying lock, retrying...");
                        NRF_LOG_FLUSH();
                        continue;
                    }
                    else
                    {
                        NRF_LOG_INFO("Locked, different user, trying lock, giving up");
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        // try unlock
        while ( true )
        {
            if ( !i2c_locked )
            {
                // not locked
                // clear vars just incase
                i2c_lock_user = 0;
                i2c_locked = false;
                NRF_LOG_INFO("Unlocked");
                return true;
            }
            else
            {
                // locked
                if ( i2c_lock_user == user_id )
                {
                    i2c_lock_user = 0;
                    i2c_locked = false;
                    NRF_LOG_INFO("Was locked, same user, unlocked");
                    return true;
                }
                else
                {
                    if ( wait )
                    {
                        NRF_LOG_INFO("Locked, different user, trying unlock, retrying...");
                        NRF_LOG_FLUSH();
                        continue;
                    }
                    else
                    {
                        NRF_LOG_INFO("Locked, different user, trying unlock, giving up");
                        return false;
                    }
                }
            }
        }
    }
}

// functions lock
static bool flashled_if_lock_aquire()
{
    return i2c_handle->LockCtl(FLASH_LED_I2C_USER_ID, true, false); // no wait on lock
}

static void flashled_if_lock_release(bool* lock)
{
    UNUSED_VAR(lock);
    i2c_handle->LockCtl(FLASH_LED_I2C_USER_ID, false, false);
}

#define flashled_if_lock_auto() \
    bool flashled_if_lock __attribute__((__cleanup__(flashled_if_lock_release))) = flashled_if_lock_aquire()

// functions lock
static bool pmu_if_lock_aquire()
{
    pmu_interface_p->Log(PWR_LOG_LEVEL_INFO, __func__);
    return pmu_interface_p->LockCtl(PMU_I2C_USER_ID, true, true);
}
static void pmu_if_lock_release(bool* lock)
{
    UNUSED_VAR(lock);
    pmu_interface_p->Log(PWR_LOG_LEVEL_INFO, __func__);
    pmu_interface_p->LockCtl(PMU_I2C_USER_ID, false, false);
}
#define pmu_if_lock_auto() bool pmu_if_lock __attribute__((__cleanup__(pmu_if_lock_release))) = pmu_if_lock_aquire()

#endif // _LOCK_H_