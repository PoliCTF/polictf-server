-- MySQL dump 10.13  Distrib 5.7.18, for Linux (x86_64)
--
-- Host: localhost    Database: polictf
-- ------------------------------------------------------
-- Server version	5.7.18-0ubuntu0.16.10.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `alerts`
--

DROP TABLE IF EXISTS `alerts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `alerts` (
  `idalert` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `recipient` int(11) unsigned DEFAULT NULL,
  `text` text COLLATE utf8_bin NOT NULL,
  `points` int(11) DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`idalert`),
  KEY `fk_recipient` (`recipient`),
  CONSTRAINT `fk_recipient` FOREIGN KEY (`recipient`) REFERENCES `team` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `alerts`
--

LOCK TABLES `alerts` WRITE;
/*!40000 ALTER TABLE `alerts` DISABLE KEYS */;
INSERT INTO `alerts` VALUES (1,NULL,'challenge1 is now open',12,'2017-05-14 13:27:57'),(8,NULL,'new hint for challenge 1, go check',12,'2017-05-14 13:41:56'),(9,NULL,'this is a very long text. We apologize for it being so long',4,'2017-05-14 13:41:56'),(14,1,'Your team solved Challenge2 (pwn)',100,'2017-06-23 11:09:41');
/*!40000 ALTER TABLE `alerts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `categories`
--

DROP TABLE IF EXISTS `categories`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `categories` (
  `idcat` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`idcat`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `categories`
--

LOCK TABLES `categories` WRITE;
/*!40000 ALTER TABLE `categories` DISABLE KEYS */;
INSERT INTO `categories` VALUES (1,'crypto'),(2,'pwn');
/*!40000 ALTER TABLE `categories` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `challenges`
--

DROP TABLE IF EXISTS `challenges`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `challenges` (
  `idchallenge` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `description` text COLLATE utf8_bin NOT NULL,
  `name` varchar(50) COLLATE utf8_bin NOT NULL,
  `file` varchar(90) COLLATE utf8_bin DEFAULT NULL,
  `category` int(11) unsigned NOT NULL,
  `flaghash` char(128) COLLATE utf8_bin NOT NULL,
  `points` int(11) NOT NULL,
  `is_flash` tinyint(1) NOT NULL DEFAULT '0',
  `opentime` timestamp NULL DEFAULT NULL,
  `minutes` int(11) unsigned DEFAULT NULL,
  `hidden` tinyint(1) NOT NULL DEFAULT '0',
  `solutions` int(11) unsigned NOT NULL DEFAULT '0',
  `nextopen` int(11) unsigned DEFAULT NULL,
  `metadata` varchar(500) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`idchallenge`),
  UNIQUE KEY `name_UNIQUE` (`name`),
  KEY `fk_challenges_categories` (`category`),
  KEY `is_flash` (`is_flash`) USING HASH,
  KEY `opentime` (`opentime`) USING BTREE,
  KEY `hidden` (`hidden`) USING HASH,
  KEY `fk_opens_idx` (`nextopen`),
  CONSTRAINT `fk_category` FOREIGN KEY (`category`) REFERENCES `categories` (`idcat`) ON UPDATE CASCADE,
  CONSTRAINT `fk_opens` FOREIGN KEY (`nextopen`) REFERENCES `challenges` (`idchallenge`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `challenges`
--

LOCK TABLES `challenges` WRITE;
/*!40000 ALTER TABLE `challenges` DISABLE KEYS */;
INSERT INTO `challenges` VALUES (1,'Get the door code to stop the squirrel invasion!','Chocolate Factory','aaaaaaaaaaaaaa.txt',1,'asfdafasgagagsaffa',5000,0,'1970-01-31 23:00:01',NULL,0,2,NULL,NULL),(2,'Challenge2 is wow','Challenge2',NULL,2,'asdafagasfa',3334,0,'1970-01-31 23:00:01',NULL,0,6,NULL,NULL);
/*!40000 ALTER TABLE `challenges` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `classifica`
--

DROP TABLE IF EXISTS `classifica`;
/*!50001 DROP VIEW IF EXISTS `classifica`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE VIEW `classifica` AS SELECT 
 1 AS `id`,
 1 AS `name`,
 1 AS `country`,
 1 AS `computed_points`*/;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `classifica_mv`
--

DROP TABLE IF EXISTS `classifica_mv`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `classifica_mv` (
  `id` int(11) NOT NULL,
  `name` varchar(25) COLLATE utf8_bin DEFAULT NULL,
  `country` varchar(20) COLLATE utf8_bin DEFAULT NULL,
  `computed_points` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='materialized view (classifica)';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `classifica_mv`
--

LOCK TABLES `classifica_mv` WRITE;
/*!40000 ALTER TABLE `classifica_mv` DISABLE KEYS */;
INSERT INTO `classifica_mv` VALUES (1,'Hanoiati','Italy',953),(2,'Team17',NULL,492);
/*!40000 ALTER TABLE `classifica_mv` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `commonstatus`
--

DROP TABLE IF EXISTS `commonstatus`;
/*!50001 DROP VIEW IF EXISTS `commonstatus`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE VIEW `commonstatus` AS SELECT 
 1 AS `idchallenge`,
 1 AS `points`,
 1 AS `open`,
 1 AS `numsolved`,
 1 AS `is_flash`*/;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `ipteams`
--

DROP TABLE IF EXISTS `ipteams`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ipteams` (
  `idteam` int(11) unsigned NOT NULL,
  `ip` varchar(30) COLLATE utf8_bin NOT NULL,
  UNIQUE KEY `uniq` (`idteam`,`ip`),
  KEY `ip` (`ip`) USING BTREE,
  KEY `team` (`idteam`) USING HASH,
  CONSTRAINT `fk_owning_team` FOREIGN KEY (`idteam`) REFERENCES `team` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ipteams`
--

LOCK TABLES `ipteams` WRITE;
/*!40000 ALTER TABLE `ipteams` DISABLE KEYS */;
INSERT INTO `ipteams` VALUES (1,'127.0.0.1');
/*!40000 ALTER TABLE `ipteams` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `solutions`
--

DROP TABLE IF EXISTS `solutions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `solutions` (
  `idsolution` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `idchallenge` int(11) unsigned NOT NULL,
  `idteam` int(11) unsigned NOT NULL,
  `ts` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `bonus` int(11) DEFAULT NULL,
  `opened` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`idsolution`),
  KEY `fk_solutions_squadre1` (`idteam`),
  KEY `fk_solutions_challenges1` (`idchallenge`),
  KEY `solvedby` (`idchallenge`,`idteam`),
  CONSTRAINT `fk_solved_challenge` FOREIGN KEY (`idchallenge`) REFERENCES `challenges` (`idchallenge`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_solving_team` FOREIGN KEY (`idteam`) REFERENCES `team` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `solutions`
--

LOCK TABLES `solutions` WRITE;
/*!40000 ALTER TABLE `solutions` DISABLE KEYS */;
INSERT INTO `solutions` VALUES (1,1,1,'2017-06-14 07:30:49',100,NULL),(2,1,2,'2017-06-14 07:31:39',100,NULL),(8,2,1,'2017-06-23 11:09:41',100,NULL);
/*!40000 ALTER TABLE `solutions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `team`
--

DROP TABLE IF EXISTS `team`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `team` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(25) COLLATE utf8_bin NOT NULL,
  `password` char(128) COLLATE utf8_bin NOT NULL,
  `email` varchar(80) COLLATE utf8_bin NOT NULL,
  `sshkey` blob,
  `ip` varchar(30) COLLATE utf8_bin NOT NULL,
  `ua` text COLLATE utf8_bin NOT NULL,
  `ts` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `confirmcode` char(50) COLLATE utf8_bin DEFAULT NULL,
  `confirmip` varchar(20) COLLATE utf8_bin DEFAULT NULL,
  `confirmua` text COLLATE utf8_bin,
  `hidden` tinyint(1) NOT NULL DEFAULT '0',
  `points` int(11) unsigned NOT NULL DEFAULT '0',
  `lastsolution` timestamp NULL DEFAULT NULL,
  `minage` int(11) DEFAULT NULL,
  `maxage` int(11) DEFAULT NULL,
  `country` varchar(20) COLLATE utf8_bin DEFAULT NULL,
  `size` int(11) DEFAULT NULL,
  `website` text COLLATE utf8_bin,
  `reset_token` varchar(45) COLLATE utf8_bin DEFAULT NULL,
  `reset_timestamp` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `email_UNIQUE` (`email`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `team`
--

LOCK TABLES `team` WRITE;
/*!40000 ALTER TABLE `team` DISABLE KEYS */;
INSERT INTO `team` VALUES (1,'Hanoiati','b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb980b1d7785e5976ec049b46df5f1326af5a2ea6d103fd07c95385ffab0cacbc86','email@example.com',NULL,'127.0.0.1','noidea','2017-05-05 19:53:41',NULL,NULL,NULL,0,50,NULL,NULL,NULL,'Italy',NULL,NULL,NULL,NULL),(2,'Team17','aasda','team17@example.com',NULL,'127.0.0.1','asda','2017-03-05 20:53:41',NULL,NULL,NULL,0,10,NULL,NULL,NULL,NULL,NULL,'Teamsite',NULL,NULL),(3,'Team18','asdafsa','lastteam@example.com',NULL,'127.0.0.1','ffff','2017-05-07 11:02:58',NULL,NULL,NULL,0,15,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `team` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'polictf'
--
/*!50003 DROP PROCEDURE IF EXISTS `SolvedChallenge` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,STRICT_ALL_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ALLOW_INVALID_DATES,ERROR_FOR_DIVISION_BY_ZERO,TRADITIONAL,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SolvedChallenge`(in teamid int,in challenge int, out pts int)
sp: BEGIN
    DECLARE cnt INT;
    DECLARE myid INT;
    #DECLARE fl INT;
    DECLARE myts TIMESTAMP;
    DECLARE bonus INT;
    DECLARE secs INT;
	declare newChallenge int;

	# has this team already solved this challenge?
    SET cnt = (SELECT COUNT(*) FROM solutions WHERE idteam=teamid and idchallenge=challenge);
    IF(cnt > 0) THEN
        set pts=-1;
        LEAVE sp;
    END IF;
    
    set myts = NOW();
    
    # add a new solution
    INSERT INTO solutions SET idchallenge=challenge, idteam=teamid,ts=myts;
    set myid = LAST_INSERT_ID();
    
    # cnt = #solves from all teams
    SET cnt = (SELECT COUNT(*) FROM solutions WHERE idchallenge=challenge and ts < myts);
    
    # ========================================================
    # Computing and setting the bonus for the current solution
    # ========================================================
    
    # base bonus for the challenge (in hundreds)
    SET pts = 100;
    
    # optional first-blood bonus (uncomment me to enable)
    #CASE cnt
    #    WHEN 0 THEN SET pts = pts * 1.15;
    #    WHEN 1 THEN SET pts = pts * 1.10;
    #    WHEN 2 THEN SET pts = pts * 1.05;
    #    ELSE SET pts=pts*1;
    #END CASE;
    
    # flash_challenges handling (OUT-OF-DATE, TO UPDATE)
    #SET fl = (SELECT is_flash FROM challenges WHERE idchallenge=challenge);
    #IF(fl = 1) THEN
    #    SET @secs=(TIMESTAMPDIFF(SECOND,(SELECT opentime FROM challenges WHERE idchallenge=challenge),NOW()));
    #    SET pts = (100*maxpts)*sqrt(log10(1+1/(pow(@secs,2))));
    #    IF(pts > maxpts) THEN
    #        SET pts=maxpts;
    #    END IF;
    #    IF(secs < 60) THEN
    #        SET pts=maxpts;
    #    END IF;
    #END IF;
    
    UPDATE solutions SET bonus = pts WHERE idsolution=myid;
    UPDATE challenges SET solutions=solutions+1 WHERE idchallenge=challenge;
    
	# ========================================================
    # Updating materialized view classifica
    # ========================================================
    
    TRUNCATE TABLE classifica_mv;
    INSERT INTO classifica_mv SELECT * from classifica;
    
	# ========================================================
    # Notifiying the team
    # ========================================================

	insert into alerts set
		text = concat("Your team solved ",
					  (select name from challenges where idchallenge=challenge),
					  " (", (select name from categories
									where idcat=(select category from challenges
																 where idchallenge=challenge)),
					  ")"),
		recipient = teamid,
		points = pts
	;
    
	# ========================================================
    # Opening the next challenge
    # ========================================================
    
	# If a challenge has been solved for the first time, automatically open the challenge challenge.nextopen (if any)
    IF(cnt=0) THEN
    
		-- Time to open a new challenge! Find the id
		set newChallenge = (select cnew.idchallenge
						        from challenges cnew inner join challenges cold
								     on cnew.idchallenge = cold.nextopen  
							    where cold.idchallenge=challenge);
                                
		IF (newChallenge is NULL) THEN
			LEAVE sp;
		END IF;
        
		-- Open the challenge
		update challenges set opentime = now() where idchallenge=newChallenge;
        
		-- Notify all teams
		insert into alerts set
			text = concat("Team ", (select name from team where id=teamid),
                          " opened new challenge: ",
						  (select name from challenges where idchallenge=newChallenge),
						  " (", (select name from categories
								     where idcat=(select category from challenges
																  where idchallenge=newChallenge)),
						  ")")
		;
    END IF;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `verify_login` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,STRICT_ALL_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ALLOW_INVALID_DATES,ERROR_FOR_DIVISION_BY_ZERO,TRADITIONAL,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `verify_login`(in nome varchar(90),in psw varchar(90),out idt int)
begin
	set idt = (select id from team where name=nome and password=sha2(psw,512));
end ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Final view structure for view `classifica`
--

/*!50001 DROP VIEW IF EXISTS `classifica`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `classifica` AS select `t`.`id` AS `id`,`t`.`name` AS `name`,`t`.`country` AS `country`,ceiling(sum(((`s`.`bonus` / 100) * `cs`.`points`))) AS `computed_points` from ((`team` `t` left join `solutions` `s` on((`t`.`id` = `s`.`idteam`))) left join `commonstatus` `cs` on((`s`.`idchallenge` = `cs`.`idchallenge`))) where (`t`.`hidden` = 0) group by `t`.`id` having (`computed_points` > 0) order by `computed_points` desc,`t`.`lastsolution`,`t`.`id` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `commonstatus`
--

/*!50001 DROP VIEW IF EXISTS `commonstatus`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `commonstatus` AS select `c`.`idchallenge` AS `idchallenge`,ceiling(((498.3 * exp((-(`c`.`solutions`) / 59.47))) + 10)) AS `points`,(`c`.`opentime` < now()) AS `open`,count(`s`.`idchallenge`) AS `numsolved`,`c`.`is_flash` AS `is_flash` from ((`challenges` `c` left join `solutions` `s` on((`s`.`idchallenge` = `c`.`idchallenge`))) left join `team` `t` on((`t`.`id` = `s`.`idteam`))) where (((`c`.`opentime` < now()) or (not(`c`.`is_flash`))) and (not(ifnull(`t`.`hidden`,false))) and (not(`c`.`hidden`))) group by `c`.`idchallenge` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-06-23 13:10:03
