const fs = require('fs');
const os = require('os');
const path = require('path');

const RRESLT = /^(.+?): (.+?) ms$/m;
const RDETLS = /^Performed (.+?) allocations of (.+?) bytes each\.$/m;




// *-FILE
// ------

function readFile(pth) {
  var d = fs.readFileSync(pth, 'utf8');
  return d.replace(/\r?\n/g, '\n');
}

function writeFile(pth, d) {
  d = d.replace(/\r?\n/g, os.EOL);
  fs.writeFileSync(pth, d);
}




// *-CSV
// -----

function writeCsv(pth, rows) {
  var cols = Object.keys(rows[0]);
  var a = cols.join()+'\n';
  for (var r of rows)
    a += [...Object.values(r)].map(v => `"${v}"`).join()+'\n';
  writeFile(pth, a);
}




// *-LOG
// -----

function readLogLine(ln, data, state) {
  state = Object.assign({allocation_size: 0, allocation_count: 0, time: 0, technique: ''}, state);
  if (!data.has('all')) data.set('all', []);
  if (RRESLT.test(ln)) {
    var [, technique, time] = RRESLT.exec(ln);
    data.get('all').push(Object.assign({}, state, {
      time: parseFloat(time),
      technique,
    }));
  }
  else if (RDETLS.test(ln)) {
    var [, allocation_count, allocation_size] = RDETLS.exec(ln);
    state.allocation_count = parseInt(allocation_count);
    state.allocation_size  = parseInt(allocation_size);
    for (var r of data.get('all')) {
      r.allocation_size  = state.allocation_size;
      r.allocation_count = state.allocation_count;
    }
  }
  return state;
}

function readLog(pth) {
  var text  = readFile(pth);
  var lines = text.split('\n');
  var data  = new Map();
  var state = {};
  for (var ln of lines)
    state = readLogLine(ln, data, state);
  return data;
}




// PROCESS-*
// ---------

function processCsv(data) {
  var a = [];
  for (var rows of data.values()) {
    for (var row of rows)
      a.push(row);
  }
  return a;
}




// MAIN
// ----

function main(cmd, log, out) {
  var data = readLog(log);
  if (path.extname(out)==='') cmd += '-dir';
  switch (cmd) {
    case 'csv':
      var rows = processCsv(data);
      writeCsv(out, rows);
      break;
    case 'csv-dir':
      for (var [graph, rows] of data)
        writeCsv(path.join(out, graph+'.csv'), rows);
      break;
    default:
      console.error(`error: "${cmd}"?`);
      break;
  }
}
main(...process.argv.slice(2));
