function computeGrade() {
  
}

function test() {console.log("si")}

async function getData() {
  try {
    const response = await fetch("./data.json");
    if (!response.ok) { throw new Error('Response status ${response.status}') }
    const result = await response.json();
    return result;
  } catch (error) {
    console.error(error.message);
    return null;
  }
}

let data = getData();

console.log(data);
