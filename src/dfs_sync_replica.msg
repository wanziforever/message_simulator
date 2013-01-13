command =: DFS_SYNC_REPLICA
loc =: 2
num =: 17
intPadding =:
sync {
       [
         chandle =: 123234
         size =: 23
         state =: 4
       ]
}

<?xml version="1.0" encoding="UTF-8"?>
<message>
  <command name="DFS_SYNC_REPLICA">
    <avp name="location">
      <value>2</value>
    </avp>
    <avp name="number">
      <value>17</value>
    </avp>
    <avp name="intPadding">
      <value></value>
    </avp>
    <avp name="syncUnit">
      <grouped>
        <chandle>2</chandle>
        <size>23</size>
        <state>4</state>
      </grouped>
    </avp>
  </command>
</message>
