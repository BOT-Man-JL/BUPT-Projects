<template>
  <el-upload action="/placeholder"
             list-type="picture"
             :on-change="onSelectFile"
             :limit="1"
             :auto-upload="false">
    <el-tag>
      {{ text }}
      <i class="el-icon-plus"></i>
    </el-tag>
  </el-upload>
</template>

<script>
  export default {
    name: 'uploadComponent',
    props: ['text', 'value'],
    methods: {
      onSelectFile(file, fileList) {
        if (fileList && fileList[0])
          this.$emit('input', fileList[0].raw);
        else
          this.$emit('input', null);
      }
    }
  }
</script>

<style scoped>
</style>
