/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"


enum MageSpells
{
    SPELL_MAGE_COLD_SNAP                         = 11958,
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_MAGE_SHEEP_FORM                        = 32820,
    SPELL_MAGE_GLYPH_OF_ETERNAL_WATER            = 70937,
	SPELL_MAGE_FLAME_ORB                         = 57752,
    SPELL_MAGE_FROSTFIRE_ORB                     = 45322,
	SPELL_MAGE_ORB_TALENT_R1                     = 84726,
	SPELL_MAGE_ORB_TALENT_R2                     = 84727,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT  = 70908,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY  = 70907,
    SPELL_MAGE_CAUTERIZE                         = 86949,
    SPELL_MAGE_CAUTERIZE_DOT                     = 87023,
    SPELL_MAGE_GLYPH_OF_BLAST_WAVE               = 62126,
	SPELL_MAGE_CONJURE_REFRESHMENT               = 42955,

};

class spell_mage_blast_wave : public SpellScriptLoader
{
    public:
        spell_mage_blast_wave() : SpellScriptLoader("spell_mage_blast_wave") { }

        class spell_mage_blast_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_blast_wave_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_GLYPH_OF_BLAST_WAVE))
                    return false;
                return true;
            }

            void HandleKnockBack(SpellEffIndex effIndex)
            {
                if (GetCaster()->HasAura(SPELL_MAGE_GLYPH_OF_BLAST_WAVE))
                    PreventHitDefaultEffect(effIndex);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_blast_wave_SpellScript::HandleKnockBack, EFFECT_2, SPELL_EFFECT_KNOCK_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_blast_wave_SpellScript();
        }
};

// Cauterize
class spell_mag_cauterize : public SpellScriptLoader
{
    public:
        spell_mag_cauterize() : SpellScriptLoader("spell_mag_cauterize") { }

        class spell_mag_cauterize_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mag_cauterize_AuraScript);

            uint32 absorbChance;

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CAUTERIZE))
                    return false;
                return true;
            }

            bool Load()
            {
                absorbChance = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Player* target = GetTarget()->ToPlayer();
                if (dmgInfo.GetDamage() < target->GetHealth() || target->HasSpellCooldown(SPELL_MAGE_CAUTERIZE) ||  !roll_chance_i(absorbChance))
                    return;    

                target->CastSpell(target, SPELL_MAGE_CAUTERIZE_DOT, true);
		        target->SetHealth(target->CountPctFromMaxHealth(40));
                target->AddSpellCooldown(SPELL_MAGE_CAUTERIZE, 0, time(NULL) + 60);

                uint32 health10 = target->CountPctFromMaxHealth(10);

                // hp > 10% - absorb hp till 10%
                if (target->GetHealth() > health10)
                    absorbAmount = dmgInfo.GetDamage() - target->GetHealth() + health10;
                // hp lower than 10% - absorb everything
                else
                    absorbAmount = dmgInfo.GetDamage();
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mag_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mag_cauterize_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mag_cauterize_AuraScript();
        }
};
class spell_mage_cold_snap : public SpellScriptLoader
{
    public:
        spell_mage_cold_snap() : SpellScriptLoader("spell_mage_cold_snap") { }

        class spell_mage_cold_snap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_cold_snap_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {

                Player* caster = GetCaster()->ToPlayer();
                // immediately finishes the cooldown on Frost spells
                const SpellCooldowns& cm = caster->GetSpellCooldownMap();
                for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

                    if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                        (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) &&
                        spellInfo->Id != SPELL_MAGE_COLD_SNAP && spellInfo->GetRecoveryTime() > 0)
                    {
                        caster->RemoveSpellCooldown((itr++)->first, true);
                    }
                    else
                        ++itr;
                }
            }

            void Register()
            {
                // add dummy effect spell handler to Cold Snap
                OnEffectHit += SpellEffectFn(spell_mage_cold_snap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_cold_snap_SpellScript();
        }
};

enum SilvermoonPolymorph
{
    NPC_AUROSALIA   = 18744,
};

// TODO: move out of here and rename - not a mage spell
class spell_mage_polymorph_cast_visual : public SpellScriptLoader
{
    public:
        spell_mage_polymorph_cast_visual() : SpellScriptLoader("spell_mage_polymorph_visual") { }

        class spell_mage_polymorph_cast_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_polymorph_cast_visual_SpellScript);

            static const uint32 PolymorhForms[6];

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                // check if spell ids exist in dbc
                for (uint32 i = 0; i < 6; i++)
                    if (!sSpellMgr->GetSpellInfo(PolymorhForms[i]))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetCaster()->FindNearestCreature(NPC_AUROSALIA, 30.0f))
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->CastSpell(target, PolymorhForms[urand(0, 5)], true);
            }

            void Register()
            {
                // add dummy effect spell handler to Polymorph visual
                OnEffectHitTarget += SpellEffectFn(spell_mage_polymorph_cast_visual_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_polymorph_cast_visual_SpellScript();
        }
};

const uint32 spell_mage_polymorph_cast_visual::spell_mage_polymorph_cast_visual_SpellScript::PolymorhForms[6] =
{
    SPELL_MAGE_SQUIRREL_FORM,
    SPELL_MAGE_GIRAFFE_FORM,
    SPELL_MAGE_SERPENT_FORM,
    SPELL_MAGE_DRAGONHAWK_FORM,
    SPELL_MAGE_WORGEN_FORM,
    SPELL_MAGE_SHEEP_FORM
};

class spell_mage_summon_water_elemental : public SpellScriptLoader
{
    public:
        spell_mage_summon_water_elemental() : SpellScriptLoader("spell_mage_summon_water_elemental") { }

        class spell_mage_summon_water_elemental_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_summon_water_elemental_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_GLYPH_OF_ETERNAL_WATER) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                // Glyph of Eternal Water
                if (caster->HasAura(SPELL_MAGE_GLYPH_OF_ETERNAL_WATER))
                    caster->CastSpell(caster, SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT, true);
                else
                    caster->CastSpell(caster, SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY, true);
            }
            
            SpellCastResult HandleCheckCast() {
                Unit *caster = GetCaster();
                if (caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                if (caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
                
                return SPELL_CAST_OK;
            }

            void Register()
            {
                // add dummy effect spell handler to Summon Water Elemental
                OnEffectHit += SpellEffectFn(spell_mage_summon_water_elemental_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_mage_summon_water_elemental_SpellScript::HandleCheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_summon_water_elemental_SpellScript();
        }
};

class spell_mage_orb : public SpellScriptLoader
{
    public:
        spell_mage_orb() : SpellScriptLoader("spell_mage_orb") { }

        class spell_mage_orb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_orb_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
				if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FLAME_ORB) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_FROSTFIRE_ORB))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                // Check Talent
                if (caster->HasAura(SPELL_MAGE_ORB_TALENT_R2) || caster->HasAura(SPELL_MAGE_ORB_TALENT_R1))
                    caster->CastSpell(caster, SPELL_MAGE_FROSTFIRE_ORB, true);
                else
                    caster->CastSpell(caster, SPELL_MAGE_FLAME_ORB, true);
            }
            

            void Register()
            {
                // add dummy effect spell handler to Flame orb
                OnEffectHit += SpellEffectFn(spell_mage_orb_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_orb_SpellScript();
        }
};

// Frost Warding
class spell_mage_frost_warding_trigger : public SpellScriptLoader
{
    public:
        spell_mage_frost_warding_trigger() : SpellScriptLoader("spell_mage_frost_warding_trigger") { }

        class spell_mage_frost_warding_trigger_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_frost_warding_trigger_AuraScript);

            enum Spells
            {
                SPELL_MAGE_FROST_WARDING_TRIGGERED = 57776,
                SPELL_MAGE_FROST_WARDING_R1 = 28332,
            };

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FROST_WARDING_TRIGGERED) || !sSpellMgr->GetSpellInfo(SPELL_MAGE_FROST_WARDING_R1))
                    return false;
                return true;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* target = GetTarget();
                if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_FROST_WARDING_R1, EFFECT_0))
                {
                    int32 chance = talentAurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue();

                    if (roll_chance_i(chance))
                    {
                        int32 bp = dmgInfo.GetDamage();
                        dmgInfo.AbsorbDamage(bp);
                        target->CastCustomSpell(target, SPELL_MAGE_FROST_WARDING_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
                        absorbAmount = 0;
                        PreventDefaultAction();
                    }
                }
            }

            void Register()
            {
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_frost_warding_trigger_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_frost_warding_trigger_AuraScript();
        }
};

class spell_mage_incanters_absorbtion_base_AuraScript : public AuraScript
{
    public:
        enum Spells
        {
            SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED = 44413,
            SPELL_MAGE_INCANTERS_ABSORBTION_R1 = 44394,
        };

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED)
                && sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_R1);
        }

        void Trigger(AuraEffect* aurEff, DamageInfo & /*dmgInfo*/, uint32 & absorbAmount)
        {
            Unit* target = GetTarget();

            if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_INCANTERS_ABSORBTION_R1, EFFECT_0))
            {
                int32 bp = CalculatePct(absorbAmount, talentAurEff->GetAmount());
                target->CastCustomSpell(target, SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_absorb : public SpellScriptLoader
{
public:
    spell_mage_incanters_absorbtion_absorb() : SpellScriptLoader("spell_mage_incanters_absorbtion_absorb") { }

    class spell_mage_incanters_absorbtion_absorb_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
    {
        PrepareAuraScript(spell_mage_incanters_absorbtion_absorb_AuraScript);

        void Register()
        {
             AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_incanters_absorbtion_absorb_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_incanters_absorbtion_absorb_AuraScript();
    }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_manashield : public SpellScriptLoader
{
public:
    spell_mage_incanters_absorbtion_manashield() : SpellScriptLoader("spell_mage_incanters_absorbtion_manashield") { }

    class spell_mage_incanters_absorbtion_manashield_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
    {
        PrepareAuraScript(spell_mage_incanters_absorbtion_manashield_AuraScript);

        void Register()
        {
             AfterEffectManaShield += AuraEffectManaShieldFn(spell_mage_incanters_absorbtion_manashield_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_incanters_absorbtion_manashield_AuraScript();
    }
};

class spell_mage_living_bomb : public SpellScriptLoader
{
    public:
        spell_mage_living_bomb() : SpellScriptLoader("spell_mage_living_bomb") { }

        class spell_mage_living_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_living_bomb_AuraScript);

            bool Validate(SpellInfo const* spell)
            {
                if (!sSpellMgr->GetSpellInfo(uint32(spell->Effects[EFFECT_1].CalcValue())))
                    return false;
                return true;
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_ENEMY_SPELL && removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                if (Unit* caster = GetCaster())
                    caster->CastSpell(GetTarget(), uint32(aurEff->GetAmount()), true, NULL, aurEff);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_mage_living_bomb_AuraScript::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mage_living_bomb_AuraScript();
        }
};
enum ConeOfColdSpells
{	  	
    SPELL_CONE_OF_COLD_AURA_R1      = 11190, // Improved Cone of Cold Rank 1 aura
    SPELL_CONE_OF_COLD_AURA_R2      = 12489, // Improved Cone of Cold Rank 2 aura
    SPELL_CONE_OF_COLD_TRIGGER_R1   = 83301, // Improved Cone of Cold Rank 1 Trigger
    SPELL_CONE_OF_COLD_TRIGGER_R2   = 83302, // Improved Cone of Cold Rank 2 Trigger
};

// 120 Cone of Cold
/// Updated 4.3.4
class spell_mage_cone_of_cold : public SpellScriptLoader
{
public:
   spell_mage_cone_of_cold() : SpellScriptLoader("spell_mage_cone_of_cold") { }

    class spell_mage_cone_of_cold_SpellScript : public SpellScript
    {
       PrepareSpellScript(spell_mage_cone_of_cold_SpellScript);

        void HandleConeOfColdScript(SpellEffIndex /*effIndex*/)
       {
           Unit* caster = GetCaster();
            if (Unit* unitTarget = GetHitUnit())
           {
               if (caster->HasAura(SPELL_CONE_OF_COLD_AURA_R1)) // Improved Cone of Cold Rank 1
                    unitTarget->CastSpell(unitTarget, SPELL_CONE_OF_COLD_TRIGGER_R1, true);
               else if (caster->HasAura(SPELL_CONE_OF_COLD_AURA_R2)) // Improved Cone of Cold Rank 2
                       unitTarget->CastSpell(unitTarget, SPELL_CONE_OF_COLD_TRIGGER_R2, true);
           }
        }

        void Register()
       {
           OnEffectHitTarget += SpellEffectFn(spell_mage_cone_of_cold_SpellScript::HandleConeOfColdScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
       }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_cone_of_cold_SpellScript();
    }
};

struct ConjureRefreshmentData
{
    uint32 minLevel;
    uint32 maxLevel;
    uint32 spellId;
};

uint8 const MAX_CONJURE_REFRESHMENT_SPELLS = 7;
const ConjureRefreshmentData _conjureData[MAX_CONJURE_REFRESHMENT_SPELLS] =
{
    { 33, 43, 92739 },
    { 44, 53, 92799 },
    { 54, 63, 92802 },
    { 64, 73, 92805 },
    { 74, 79, 74625 },
    { 80, 84, 92822 },
    { 85, 85, 92727 }
};

class spell_mage_conjure_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_conjure_refreshment() : SpellScriptLoader("spell_mage_conjure_refreshment") { }

        class spell_mage_conjure_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_conjure_refreshment_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                    if (!sSpellMgr->GetSpellInfo(_conjureData[i].spellId))
                        return false;
                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint8 level = GetHitUnit()->getLevel();
                for (uint8 i = 0; i < MAX_CONJURE_REFRESHMENT_SPELLS; ++i)
                {
                    ConjureRefreshmentData const& spellData = _conjureData[i];
                    if (level < spellData.minLevel || level > spellData.maxLevel)
                        continue;
                    GetHitUnit()->CastSpell(GetHitUnit(), spellData.spellId);
                    break;
                }
            }


            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_mage_conjure_refreshment_SpellScript();
        }
};
void AddSC_mage_spell_scripts()
{
    new spell_mage_blast_wave();
    new spell_mage_cold_snap();
    new spell_mage_frost_warding_trigger();
    new spell_mage_incanters_absorbtion_absorb();
    new spell_mage_incanters_absorbtion_manashield();
    new spell_mage_polymorph_cast_visual();
    new spell_mage_summon_water_elemental();
	new spell_mage_orb();
    new spell_mage_living_bomb();
    new spell_mage_cone_of_cold();
    new spell_mag_cauterize();
	new spell_mage_conjure_refreshment();
}
