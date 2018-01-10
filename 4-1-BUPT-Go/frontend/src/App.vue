<template>
  <div id="app">
    <el-col :span="7">
      .
    </el-col>
    <el-col :span="10">
      <router-view />
    </el-col>
    <el-col :span="7">
      .
    </el-col>
  </div>
</template>

<script>
export default {
  name: 'app'
}
</script>

<style scoped>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  color: #2c3e50;
}
</style>
