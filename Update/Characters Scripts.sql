/*Characters.sql Updates
 *Myendras Characters Updates
 *LastUpdate: Phailure
 *http://wow.urnaweb.cz/wotlk/web/index.php
 */


ALTER TABLE `characters`
	CHANGE COLUMN `drunk` `drunks` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `watchedFaction`;

CREATE TABLE fake_items (
	`guid` INT(11) NOT NULL,
	`fakeEntry` INT(11) NOT NULL
)

ALTER TABLE character_glyphs CHANGE glyph1 glyph1 smallint(5) unsigned DEFAULT '0';