Freebox4-NG Ultimate
Le plugin Freebox pour Qt Creator, modernisé pour développer, diagnostiquer et tester les applications QML des Players Freebox.

Plugin Freebox officiel · Branche officielle Qt Creator 18 · SDK Freebox
État : prototype avancé / validation en cours. La version Ultimate v4 cible Qt Creator 20.0.2 sous Windows x64. Le binaire a été compilé et testé avec un environnement Windows de test et des Players simulés, mais son fonctionnement complet sur les Freebox Révolution et Devialet physiques n'est pas encore certifié.

Pourquoi Freebox4-NG ?
Développer une application pour les Players Freebox ne se limite pas à écrire du QML : il faut déployer sur un matériel embarqué, identifier les problèmes de focus à la télécommande, comprendre les erreurs réseau, mesurer les performances et vérifier les régressions entre deux builds.

Freebox4-NG Ultimate reprend la base du plugin Freebox et cherche à transformer Qt Creator en un environnement de travail intégré : écriture → validation → lancement → observation → test → correction.

Le projet poursuit trois objectifs :
1. Prolonger la compatibilité du plugin Freebox avec un Qt Creator moderne, sans changer artificiellement le runtime QML des Players.
2. Rendre les problèmes reproductibles, grâce à la télémétrie, aux scénarios de télécommande, au débogage et aux comparaisons de traces.
3. Ouvrir la voie à une assistance IA contrôlée, via les outils natifs MCP de Qt Creator et, à terme, des agents capables de proposer, tester et comparer des corrections sous supervision humaine.
Le développement est notamment guidé par les besoins rencontrés sur ReDeFin, client Jellyfin pour Freebox Révolution et Devialet. Freebox4-NG n'est toutefois pas réservé à ReDeFin ni aux applications Jellyfin.
Compatibilité et prérequis
Élément	Cible actuelle
IDE	Qt Creator 20.0.2
Système hôte	Windows x64
Plugin	Freebox4.dll, extension native de Qt Creator
Compilation	CMake, Visual Studio 2022 / outils MSVC x64, Qt 6.11.x adapté au SDK Qt Creator
Applications cibles	QML / JavaScript pour le SDK Freebox, notamment Qt Quick 2.15 et Qt Multimedia 5.15 selon le Player
Matériels visés	Freebox Révolution et Player Freebox Devialet, en mode développeur
Validation matérielle	En attente de tests complets sur les Players physiques


À ne pas confondre : Qt Creator 20 utilise un environnement de développement récent, mais il ne transforme pas automatiquement les bibliothèques Qt disponibles sur la Freebox en Qt 6. Le plugin doit précisément aider à détecter ces incompatibilités.
Plugin officiel de Free et Freebox4-NG : quelles différences ?
La comparaison ci-dessous prend comme référence la branche officielle qtcreator-18.0.0, et non uniquement l'ancien README du dépôt principal historiquement lié à Qt Creator 4.3.0. La branche Qt Creator 18 de Free annonce déjà le fonctionnement de Run, Debug et Profiling : ces fonctions ne sont donc pas présentées comme des exclusivités de Freebox4-NG.
Fonction	Plugin Free, branche Qt Creator 18	Freebox4-NG Ultimate v4
Environnement Qt Creator	Qt Creator 18	Qt Creator 20.0.2, Windows x64
Projet .fbxproject et manifeste Freebox	Oui	Conservés, avec validations supplémentaires
Découverte / sélection des Players	Oui	Conservées, ajout manuel par IP et diagnostics de sélection
Lancement et arrêt à distance	Oui	Conservation du flux natif, travail sur le remplacement fiable de session main
Débogage QML/JavaScript	Oui	Débogage natif et fonctions d'inspection complémentaires
QML Profiler	Oui	Profiler natif et analyse comparative de fichiers .qtd
Validation orientée SDK Freebox	Non revendiquée ici comme fonctionnalité officielle équivalente	Analyse des imports, API incompatibles, ressources et manifeste
Diagnostics corrélés	Pas d'équivalent spécialisé documenté dans la référence examinée	Chronologie de session, format FBX_DIAG v3, filtres et export
Télécommande / scénarios rejouables dans l'IDE	Pas d'équivalent spécialisé documenté	HID/RUDP et scénarios JSON, validés avec simulateur
Inspection QML et historique du focus	Inspection via débogueur natif	Vue Live QML and Focus, historique et corrélation avec événements instrumentés
Tests de parcours et comparaison de captures	Pas d'équivalent spécialisé documenté	Assertions, comparaison de scénarios et captures externes
Outils Freebox pour MCP / agents IA	Pas d'ensemble d'outils freebox_* équivalent documenté	39 outils exposés dans le code v4, intégration agent encore partiellement validée


Important : « pas d'équivalent documenté » ne veut pas dire que Qt Creator ou le plugin officiel sont dépourvus de toute capacité comparable. Plusieurs fonctions, dont le débogage, le profilage et l'infrastructure MCP, sont fournies par Qt Creator lui-même. Freebox4-NG s'appuie sur elles et ajoute une couche spécifique à l'écosystème Freebox.
Fonctionnalités de Freebox4-NG Ultimate
1. Développement et cycle de lancement
- Ouverture des projets .fbxproject, lecture du manifest.json et empaquetage .fbxqml destiné au flux FreeStore.
- Découverte SSDP des Players, sélection du kit et ajout d'une cible par adresse IP.
- Lancement avec Ctrl+R, arrêt de session et tentative de remplacement maîtrisé d'une session main déjà présente.
- Vérifications préalables pour éviter le déploiement d'un projet manifestement invalide.
2. Débogage, inspection et performance
- Débogueur QML/JavaScript natif via F5 : points d'arrêt, variables, pile d'appels et pas à pas.
- Inspecteur d'objets et propriétés accessibles via le débogueur ; panneau Live QML and Focus pour les observations effectivement disponibles.
- Analyse des changements de focus à partir d'événements transmis par l'application et corrélation avec les commandes de la télécommande.
- Capture et comparaison de traces QML Profiler .qtd : événements JavaScript, coûts observables et métriques mémoire lorsqu'elles existent dans la trace.
- Comparaison de révisions Git et de sessions pour aider à identifier les régressions.
Le suivi des changements de propriétés n'est pas encore un observateur universel en temps réel : il dépend des données fournies par le débogueur et de l'instrumentation du projet. Les valeurs absentes sont signalées comme telles.
3. Centre de diagnostic
- Journal de session configurable : événements QML/JS, lancement, réseau, télécommande et lecture multimédia, dans la limite de la télémétrie disponible.
- Filtrage et export de diagnostics, masquage des secrets et identifiants de corrélation pseudonymisés.
- Format FBX_DIAG v3 pour relier les étapes d'un même parcours sans exposer les identifiants et jetons utilisateur.
- Éclairage ciblé sur les problèmes typiques d'applications multimédias : authentification, réponses HTTP, redirections, DNS/TLS, négociation de lecture, remux, transcodage, focus et navigation.
Ces diagnostics n'interceptent pas magiquement tout le trafic : certains signaux doivent être explicitement émis par l'application, et les détails réseau ou multimédias ne sont exploitables que lorsqu'ils sont observables.
4. Télécommande et tests reproductibles
- Envoi de touches via HID/RUDP lorsque le Player et son service le permettent.
- Enregistrement, import et export de scénarios JSON ; rejeu avec répétition, pause, annulation et limites de durée.
- Assertions sur le focus, le chargement d'une page, les réponses réseau, les propriétés observées ou l'absence d'erreurs critiques.
- Résultats conservés et comparaison de parcours, y compris entre deux configurations de Players lorsque les données sont disponibles.
- Comparaison de captures PNG/JPEG fournies localement, avec masques, tolérances et détection d'une image presque noire.
Limite actuelle : la comparaison d'images ne déclenche pas de capture native automatique de l'écran d'une Freebox physique.
5. Assistance IA et MCP
Freebox4-NG étend l'environnement MCP de Qt Creator avec des outils spécialisés freebox_* : sélection du Player, validation, lancement, attente de résultats, lecture bornée de diagnostics, analyse de traces, télécommande, scénarios et inspection QML.

Le projet conserve 39 noms d'outils Freebox dans la v4. Leur fonctionnement a été exercé avec un client de test ; le cycle complet avec Codex et un agent ACP reste à confirmer. Les appels externes ne constituent pas une autorisation d'action automatique.

Principes de sécurité :
- Autorisation explicite pour un projet donné et outils désactivables individuellement.
- Confirmation locale pour les actions sensibles telles que lancement, arrêt, télécommande ou lecture de source.
- Accès borné aux fichiers et traces du projet, masquage de données sensibles et refus hors périmètre.
- Serveur MCP limité à localhost pour les usages locaux recommandés.
- Aucune dépendance obligatoire à un service IA pour utiliser les fonctions de base du plugin.
Installation rapide
1. Installer Qt Creator 20.0.2 Windows x64 et les dépendances adaptées à cette version.
2. Fermer Qt Creator et sauvegarder une éventuelle ancienne Freebox4.dll.
3. Depuis l'archive binaire Ultimate v4, placer lib/qtcreator/plugins/Freebox4.dll dans le répertoire équivalent de l'installation Qt Creator, sans modifier les autres DLL.
4. Relancer Qt Creator puis vérifier l'activation de Freebox dans Aide → À propos des extensions.
5. Ouvrir un fichier .fbxproject, activer le mode développeur sur le Player, puis utiliser Outils → Freebox SDK → Players and Active Target. Une adresse IP peut être ajoutée manuellement.
6. Démarrer l'application avec Ctrl+R.
Pour un retour arrière, fermer Qt Creator et restaurer la DLL précédemment sauvegardée. Ne pas installer cette DLL sur une autre version de Qt Creator sans compilation et test adaptés.
Compiler depuis les sources
La compilation de maintenance utilise notamment CMake, Visual Studio 2022, les SDK Qt / Qt Creator correspondants et le script PowerShell fourni dans les sources :
.\scripts\Build-Windows-x64.ps1 `
  -QtPrefix C:\Qt\6.11.2\msvc2022_64 `
  -QtCreatorSdk C:\Qt\Creator20-sdk `
  -QtCreatorSource C:\Qt\Creator20-src
Ajouter -BuildTests pour construire aussi les bancs de test. L'archive binaire v4 et les guides décrivent l'environnement de compilation employé ; la publication de sources v4 exactement correspondantes doit accompagner une release publique du binaire.
Premiers pas dans Qt Creator
Action	Accès
Lancer / relancer	Ctrl+R
Déboguer QML / JavaScript	F5
Profiler	Analyser → QML Profiler
Sélectionner un Player	Outils → Freebox SDK → Players and Active Target
Vérifier le projet	Validate Freebox Project
Créer un paquet FreeStore	Ctrl+Alt+P
Ouvrir la télécommande et les scénarios	Ctrl+Alt+R
Consulter les événements	Session Diagnostics


Pour les tests automatisés, privilégier une copie Git isolée du projet, un scénario de référence reproductible et une revue humaine avant toute correction proposée par un agent.
Historique des versions
Ultimate v4 · 13 novembre 2026 · Prototype avancé
- Nouvelle fenêtre Live QML and Focus : arborescence observée, recherche, historique et export.
- Corrélation des transitions de focus avec les commandes HID acquittées.
- Assertions supplémentaires sur événements réseau, propriétés, fin d'opération et erreurs critiques ; récupération facultative.
- Conservation et comparaison des résultats de scénarios, y compris sur des cibles distinctes.
- Comparaison de captures externes PNG/JPEG avec masques, tolérances et rapport d'écart.
- Lecture incrémentale de la chronologie MCP ; conservation des 39 outils de la v3.
- Mesures de mémoire à partir de traces instrumentées, lorsqu'elles sont présentes.
- Métadonnées SDK additionnelles pour fbx.async (StableTimer, TimeQueue).
Ultimate v3 · 13 novembre 2026
- Télécommande HID/RUDP intégrée à Qt Creator, avec acquittements et retransmission.
- Enregistrement, export et rejeu borné de scénarios JSON.
- Assertions et chronologie corrélant touches, focus, diagnostics et résultats.
- Compléments de descriptions du SDK fbx.application pour le modèle de code.
- 10 outils MCP supplémentaires, portant l'ensemble à 39.
- Tests simulés de commandes, refus, annulation, rejeu et instrumentation QML.
Ultimate v2 · novembre 2026
- Stabilisation de la transition Stop / débogueur et de la résolution des sources.
- Inspection QML native avec instantanés bornés et lecture MCP.
- Diagnostics FBX_DIAG v3 corrélés : interface, réseau et multimédia.
- Suivi d'attentes MCP, vérification d'étapes, comparaison Git locale et de diagnostics.
- Premiers outils de comparaison de traces et essais d'endurance.
Fondations Freebox4-NG · travaux antérieurs
- Adaptation du plugin natif aux API de Qt Creator 20.0.2.
- Conservation des projets Freebox, de la découverte réseau, du lancement/débogage et du packaging.
- Préparation d'un socle de validation et d'automatisation adapté aux applications QML embarquées.
Les entrées ci-dessus décrivent l'historique du développement, pas quatre versions publiques officiellement testées sur tous les matériels. Pour chaque livraison, consulter le rapport de tests correspondant avant de qualifier une fonctionnalité de prête pour la production.

Tests réalisés et limites connues
Résultats documentés pour la v4 :
- Binaire Freebox4.dll Windows x64 produit et chargé dans Qt Creator 20.0.2 sous WinPE x64 / QEMU.
- 22 tests réussis sous Linux (Qt Test/CTest) ; 20 réussites sous Windows simulé, avec un test ignoré.
- Ouverture de projets Freebox, déploiement simulé, kit, SSDP, Ctrl+R et remplacement de session exercés sur un service de développement simulé.
- Débogage F5 : point d'arrêt, variable JavaScript, pile d'appels, pas à pas et arrêt vérifiés avec des fixtures Qt 5.15.2.
- Capture et comparaison d'une trace QML native .qtd dans la session ; réouverture native après redémarrage non validée.
- 60 cycles d'endurance et 240 tentatives réalisés sur la DLL v4 livrée avec un Player simulé.
- Essai de connexion de Codex au serveur MCP : initialisation atteinte, énumération des outils expirée ; ACP non validé.
Ce que ces résultats ne prouvent pas :
- Le fonctionnement complet sur une Freebox Révolution ou Devialet réelle et sur un poste Windows 11 complet.
- Le succès des commandes HID sur les firmwares réels ou la stabilité de toutes les reconnexions.
- L'observation universelle et continue de chaque changement de propriété ou binding QML.
- L'exécution autonome de corrections par un agent IA, de bout en bout.
- L'absence de fuite mémoire : les métriques portent sur le processus Qt Creator entier et sur les scénarios testés.
Les résultats détaillés et leurs environnements sont conservés dans les documents de qualification de chaque version. La qualité de cette extension doit être établie par des tests sur matériels réels, et non uniquement par la présence de code ou la réussite de simulations.
Ambitions et feuille de route
Prochaine étape : fiabilisation sur les vrais Players
- Valider Ctrl+R, Stop et F5 sur Révolution et Devialet, avec reconnexions et changements de session répétés.
- Vérifier les commandes HID, le focus réel, les traces QML et le Profiler sur leurs firmwares.
- Corriger les problèmes de session et de persistance identifiés sur Windows 11 complet.
- Industrialiser des tests de non-régression reproductibles et des publications avec sources correspondantes.
Diagnostic plus profond
- Reconstituer les parcours navigation → réseau → Jellyfin / backend → lecture, lorsque les événements sont accessibles.
- Détecter plus rapidement écrans noirs, blocages de focus, requêtes interrompues, régressions de fluidité et consommation mémoire inhabituelle.
- Mieux comparer les performances entre commits et entre modèles de Players, en signalant les métriques non observables.
- Élargir progressivement les descriptions des API Freebox documentées pour l'éditeur et le validateur.
Vision Ultimate : un assistant de développement supervisé
- Faire dialoguer Qt Creator, MCP, ACP et un agent de code autour de diagnostics et de scénarios reproductibles.
- Proposer des corrections dans une branche ou un worktree isolé, sans modifier silencieusement la branche de publication.
- Valider un changement, relancer le Player, rejouer le scénario, comparer les résultats et présenter les preuves au développeur.
- Envisager une analyse de régression plus automatisée, y compris le rapprochement Git entre une version fonctionnelle et une version défaillante.
Cette vision est une feuille de route, pas une fonctionnalité autonome déjà livrée. Les confirmations humaines, limites d'accès et tests restent centraux.
Contributions, sécurité et confidentialité
Les retours les plus utiles sont ceux qui incluent le modèle de Player, le firmware, la version de Qt Creator, les étapes de reproduction et un journal anonymisé. Pour les problèmes multimédias, joindre le mode de lecture, les codecs et le résultat observé peut aider.

N'incluez jamais de jetons Jellyfin, mots de passe, URLs privées, identifiants d'abonnés ou données personnelles dans un ticket public. Les outils de diagnostic et MCP doivent rester limités aux projets et environnements autorisés. Un port de débogage QML ne doit pas être exposé à Internet.

Les contributions peuvent porter sur la compatibilité Qt Creator, le SDK Freebox, le validateur, les tests, les traductions et la documentation. Merci de joindre une méthode de reproduction et, dans la mesure du possible, un test associé.
Origine, licence et crédits
Freebox4-NG Ultimate est une initiative communautaire indépendante, fondée sur le plugin Freebox pour Qt Creator. Ce projet n'est ni édité, ni certifié, ni officiellement soutenu par Free ou Qt.
- Plugin original : Freebox SAS et contributeurs.
- Référence moderne officielle : branche qtcreator-18.0.0.
- Documentation de la plateforme : SDK Freebox Player.
- Projet ayant motivé des cas d'usage : ReDeFin.
- Modernisation communautaire : Laborantine / Freebox4-NG Ultimate.
Le plugin Freebox original est distribué sous LGPL-2.1 et les sources reprises doivent conserver les mentions de droits d'auteur et les licences applicables. Certains composants tiers, notamment le code HID Freebox, possèdent leur propre notice. Conserver les fichiers COPYING, AUTHORS et les notices tierces lors de la publication ; ne pas remplacer ces licences par celle d'un autre projet sans analyse préalable.

Les marques Freebox, Free et Qt appartiennent à leurs titulaires respectifs.
Freebox4-NG Ultimate : transformer une session de débogage isolée en un parcours de développement mesurable, reproductible et, à terme, assisté par IA.
