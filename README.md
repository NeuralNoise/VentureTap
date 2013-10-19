VentureTap
==========

In a nutshell
-------------

VentureTap was a private invite-only social network for angel investors and venture capitalists. It was developed using Drupal CMS and a whole lot of custom PHP code. Most of the features were implemented over a period of eight months in 2008-2009, when the first invites for www.venturetap.com were sent out. It gained around 50 users, spanning 12 countries and investment ranges from under $10K to over $20M. The site ultimately did not gain enough traction and was eventually put on life support.

> NOTE: The committed code represents the VentureTap custom module only and not the entirety of a Drupal installation.

What's the point?
-----------------

In 2007 angel investing was a somewhat obscure practice, with the funding landscape dominated by a multitude of VC firms. Deal syndication was starting to appear, bridging the gap between traditional angels and VCs. VentureTap was designed to foster collaboration between angel investors and provide a reliable source of deal flow between accredited investors. Support for venture capitalists was secondary, as they were assumed to have large pre-existing networks.

The main competitor in this space at the time was AngelSoft (currently Gust.com), however there was no social component to their offering and this was the void that VentureTap intended to fill. It offered all of the social networking staples:
- Activity feeds
- Profiles
- Connections
- Open and moderated groups
- Deal sharing with attachments
- Private messaging
- Invites
- Advanced search

I quit my job in May 2008 to work on VentureTap full-time and continued working on it in the evenings after going back to full-time employment in early 2009. The end result was not a success, however it was an extremely valuable personal experience with taking a project from initial design, through implementation (in a new language and framework, no less) and all the way to delivery and maintenance. On the whole, I have no regrets about undertaking this venture.

Lessons learned
---------------

Despite my expertise with Microsoft technologies and .NET specifically, implementing a social network from scratch in ASP.NET would have proven to be an extremely time-consuming and likely futile endeavor. I turned to open source in order to avoid reinventing the social networking wheels. Drupal, with its promise of modular software design and a wealth of custom modules, was selected as the framework on which VentureTap was to be built.

For reasons discussed below, if I were faced with a similar decision today, I would not be considering Drupal - most likely, Python + Django would be the winning combination. At the time, however, Django was not nearly as evolved and the community was much smaller, so it was not even on my radar. Under the circumstances, I feel that I made the right decision as there were indeed enough building blocks to build a social network. The problem lay with all the glue that was needed to make a custom, functional web application.

**Drupal:** The learning curve for Drupal was very steep. It was easy enough to throw together a few modules and get a semi-functioning website, however stepping away from default behavior required a deep understanding of Drupal guts and the module infrastructure. Drupal 5 did not use the object-oriented features of PHP, instead relying on a hook mechanism to override or supplement existing behavior. In the end, most of the time developing in Drupal felt like gluing together modules that were never meant to be compatible and squashing bugs that inevitably resulted from veering off the default path. The CMS roots of Drupal were both a huge benefit (from reusability perspective) and a huge liability (from customizability perspective).

**PHP:** An extremely easy language to pick up and use. Due to limitations and conventions of Drupal code base, I was stuck with procedural PHP and did not make use of any object-oriented features. My overall impression of the standard libraries was that of total chaos and disorganization. It seems no attention was paid to enforcing any kind of naming conventions, the whole thing felt like one giant historical artifact. Lack of namespaces did not help matters. By itself, the language was not really a roadblock - it was easy enough to memorize the most common operations, however with so many "cleaner" alternatives out there, it's hard to imagine myself picking PHP for a new project.

**MySQL:** Not much to say here. I picked MySQL, because PostgreSQL support in Drupal was rather sketchy. In terms of the feature set, MySQL trailed both PostgreSQL and SQL Server, however it was fast and filled my simple needs adequately. I chose the InnoDB engine over MyISAM for referential integrity.

The biggest non-technical takeaway for me was that development in a vacuum can sink a project that may have otherwise stood a chance. Confusing secrecy for competitive advantage, I was very guarded about sharing my thoughts with other people and in fact did not solicit feedback until after the first cut of the code was released. Having read books on angel investing, I thought I understood the space - if only I had been aware of the concepts of lean software development! If nothing else, I gained appreciation for transparency and communication, albeit too late to make a difference for VentureTap.
