CREATE (Bob:User {name:'Bob', music: 'reggae', hobby: 'soccer'})
CREATE (Peter:User {name:'Peter'})
CREATE (Anna:User {name:'Anna'})
CREATE (Amy:User {name:'Amy'})
CREATE 
    (Bob)<-[:knows {since: '2012'}]-(Peter),
    (Bob)-[:knows{since: '2008'}]->(Amy),
    (Anna)-[:knows{since: '2001'}]->(Peter),
    (Anna)-[:knows{since: '1999'}]->(Amy),
    (Peter)-[:knows{since: '2020'}]->(Amy)
