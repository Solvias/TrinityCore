/*Auth.sql Updates
 *Myendras Auth Updates
 *LastUpdate: Phailure
 *http://wow.urnaweb.cz/wotlk/web/index.php
 */

-- SET @Realmnam = [FUN] Myendras;

CREATE TABLE dummy (
	`time` INT(10) UNSIGNED NOT NULL,
	`realm` INT(10) UNSIGNED NOT NULL,
	`type` TINYINT(3) UNSIGNED NOT NULL,
	`level` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`string` TEXT NULL COLLATE 'latin1_swedish_ci'
)


-- INSERT INTO `realmlist` (`id`, `name`, `address`, `port`, `icon`, `flag`, `timezone`, `allowedSecurityLevel`, `population`, `gamebuild`) VALUES (1, '@Realmname', '188.165.206.170', 8087, 0, 0, 1, 0, 0, 12340);

